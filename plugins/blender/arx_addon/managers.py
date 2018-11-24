# Copyright 2015-2017 Arx Libertatis Team (see the AUTHORS file)
#
# This file is part of Arx Libertatis.
#
# Arx Libertatis is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# Arx Libertatis is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with Arx Libertatis. If not, see <http://www.gnu.org/licenses/>.

import importlib
import sys
import os

if "bpy" in locals():
    importlib.reload(sys.modules["arx_addon.common"])
    importlib.reload(sys.modules["arx_addon.dataCommon"])
    importlib.reload(sys.modules["arx_addon.dataDlf"])
    importlib.reload(sys.modules["arx_addon.dataFtl"])
    importlib.reload(sys.modules["arx_addon.dataFts"])
    importlib.reload(sys.modules["arx_addon.dataLlf"])
    importlib.reload(sys.modules["arx_addon.dataTea"])
    importlib.reload(sys.modules["arx_addon.files"])
    importlib.reload(sys.modules["arx_addon.lib"])

import bpy
import bmesh
import math
import mathutils
from mathutils import Vector,Quaternion

from .lib import ArxIO
from .dataDlf import DlfSerializer, DlfData
from .dataFtl import FtlSerializer, FtlData, FtlMetadata, FtlVertex, FtlFace
from .dataFts import FtsSerializer
from .dataLlf import LlfSerializer
from .dataTea import TeaSerializer
from .common import *
from .files import *

correctionMatrix = \
    mathutils.Matrix.Rotation(math.radians(180), 4, 'Z') @ \
    mathutils.Matrix.Rotation(math.radians(-90), 4, 'X')


def triangulate(bm):
    while True:
        nonTris = [f for f in bm.faces if len(f.verts) > 3]
        if nonTris:
            nt = nonTris[0]
            pivotLoop = nt.loops[0]
            allVerts = nt.verts
            vert1 = pivotLoop.vert
            wrongVerts = [vert1, pivotLoop.link_loop_next.vert, pivotLoop.link_loop_prev.vert]
            bmesh.utils.face_split(nt, vert1, [v for v in allVerts if v not in wrongVerts][0])

            for seq in [bm.verts, bm.faces, bm.edges]:
                seq.index_update()
        else:
            break
    return bm


def strip_wires(bm):
    [bm.verts.remove(v) for v in bm.verts if v.is_wire]
    [bm.verts.remove(v) for v in bm.verts if not v.link_faces[:]]
    [bm.edges.remove(e) for e in bm.edges if not e.link_faces[:]]
    [bm.faces.remove(f) for f in bm.faces if len(f.edges) < 3]
    for seq in [bm.verts, bm.faces, bm.edges]: seq.index_update()
    return bm


class ArxException(Exception):
    """Common exception thrown by this addon"""
    pass

class InconsistentStateException(Exception):
    """Thrown if data supposed to be added to existing data does not match"""
    pass



def arx_pos_to_blender_for_model(pos):
    """x=>x; y=>-z; z=>y"""
    return Vector((pos[0], pos[2], -pos[1]))

import itertools

class ArxObjectManager(object):
    def __init__(self, ioLib, dataPath):
        self.log = logging.getLogger('ArxObjectManager')
        self.ioLib = ioLib
        self.dataPath = dataPath
        self.ftlSerializer = FtlSerializer()

    def validateAssetDirectory(self):
        if not os.path.isdir(self.dataPath):
            raise ArxException("Arx assert directory path [" + self.dataPath + "] is not a directory")
        
        if not os.path.exists(self.dataPath):
            raise ArxException("Arx assert directory path [" + self.dataPath + "] does not exist")

    def validateObjectName(self, name):
        if len(name.encode('utf-8')) > 63:
            raise ArxException("Name ["+name+"] too long to be usesd as blender object name")

    def analyzeFaceData(self, faceData):
        # find weird face data
        facesToDrop = []

        seenFaceVerts = {}
        for i, face in enumerate(faceData):
            for foo in itertools.permutations(face.vids):
                if foo in seenFaceVerts:
                    seen = seenFaceVerts[foo]
                    facesToDrop.append(seen)
                    break

            seenFaceVerts[face.vids] = i

        if len(facesToDrop) > 0:
            log.debug("Dropping faces: " + str(facesToDrop))

        return facesToDrop

    def createBmesh(self, context, vertData, faceData) -> bmesh.types.BMesh:

        facesToDrop = self.analyzeFaceData(faceData)

        bm = bmesh.new()

        arxFaceType = bm.faces.layers.int.new('arx_facetype')
        arxTransVal = bm.faces.layers.float.new('arx_transval')

        uvData = bm.loops.layers.uv.verify()
        for i, (position, normal) in enumerate(vertData):
            vertex = bm.verts.new(arx_pos_to_blender_for_model(position))
            vertex.normal = normal
            vertex.index = i

        bm.verts.ensure_lookup_table();

        for i, (vids, uvs, texid, facetype, transval, normal) in enumerate(faceData):

            if i in facesToDrop:
                continue

            # if texid < 0:
            #    continue

            faceVerts = [bm.verts[v] for v in vids]
            evDict = {}
            try:
                face = bm.faces.new(faceVerts)
                face.index = i
            except ValueError:
                self.log.debug("Extra face")

            face.normal = Vector(normal)

            if texid >= 0:
                face.material_index = texid

            face[arxFaceType] = facetype
            face[arxTransVal] = transval

            for j, loop in enumerate(face.loops):
                u, v = uvs[j]
                loop[uvData].uv = u, 1.0 - v

        bm.faces.ensure_lookup_table();

        bm.edges.index_update()
        bm.edges.ensure_lookup_table()

        return bm

    def createObject(self, context, bm, data, canonicalId, collection) -> bpy.types.Object:

        idString = "/".join(canonicalId);
        self.validateObjectName(idString)
        mesh = bpy.data.meshes.new(idString)
        bm.to_mesh(mesh)

        #armatureObj = self.createArmature(canonicalId, bm, data.groups)

        bm.free()

        if bpy.context.active_object:
            bpy.ops.object.mode_set(mode='OBJECT')

        obj = bpy.data.objects.new(name=mesh.name, object_data=mesh)
        #armatureObj.parent = obj # this created a dependecy recursion, so i commented it

        obj['arx.ftl.name'] = data.metadata.name
        obj['arx.ftl.org'] = data.metadata.org

        for i, (name, origin, indices, parentIndex) in enumerate(data.groups):
            grp = obj.vertex_groups.new(name="grp:" + str(i).zfill(2) + ":" + name)
            # XXX add the origin to the group ?
            for v in indices:
                grp.add([v], 0.0, 'ADD')

        for i, (name, indices) in enumerate(data.sels):
            grp = obj.vertex_groups.new(name="sel:" + str(i).zfill(2) + ":" + name)

            for v in indices:
                grp.add([v], 0.0, 'ADD')

        for (name, vidx) in data.actions:
            action = bpy.data.objects.new(name, None)
            action.parent = obj
            action.parent_type = 'VERTEX'
            action.parent_vertices = [vidx, 0, 0]  # last two are ignored
            action.show_name = True
            collection.objects.link(action)

            if name.lower().startswith("hit_"):
                radius = int(name[4:])
                action.empty_display_type = 'SPHERE'
                action.empty_display_size = radius
                action.lock_rotation = [True, True, True]
                action.lock_scale = [True, True, True]
            else:
                action.scale = [3, 3, 3]
                
            
        #armatureModifier = obj.modifiers.new(type='ARMATURE', name="Skeleton")
        #armatureModifier.object = armatureObj


        # for toDeSel in scene.objects: #deselect all first just to be sure
        #    toDeSel.select = False

        collection.objects.link(obj)
        #FIXME properly bind bones to mesh via vertex groups
        #obj.select = True
        #armatureObj.select = True
        #bpy.context.scene.objects.active = armatureObj #select both the mesh and armature and set armature active
        #bpy.ops.object.parent_set(type='ARMATURE_AUTO') # ctrl+p and automatic weights
        #bpy.context.scene.objects.active = obj
        #armatureObj.select = False

        #bpy.ops.object.mode_set(mode='EDIT', toggle=False)  # initialises UVmap correctly


        # TODO 2.8
        # mesh.uv_textures.new()
        for m in data.mats:
            mat = createMaterial(self.dataPath, m)
            obj.data.materials.append(mat)
            
        #bpy.ops.object.mode_set(mode='OBJECT', toggle=False)
        return obj

    def createArmature(self, canonicalId, bm, groups) -> bpy.types.Object:

        amtname = "/".join(canonicalId)
        origin = (0, 0, 0)

        # Create armature and object
        amt = bpy.data.armatures.new(amtname)
        amt.draw_type = 'WIRE'
        amtobject = bpy.data.objects.new(amtname + "-amt", amt)
        # amtobject.show_x_ray = True
        amtobject.location = origin
        # ob.show_name = True

        # Link object to scene and make active
        bpy.context.scene.collection.link(amtobject)
        bpy.context.scene.collection.active = amtobject
        amtobject.select = True

        bpy.ops.object.mode_set(mode='EDIT')

        for i, (name, origin, indices, parentIndex) in enumerate(groups):
            bGrpName = "grp:" + str(i).zfill(2) + ":" + name

            bone = amt.edit_bones.new(bGrpName)
            bone.head = bm.verts[origin].co
            bone.tail = bm.verts[origin].co + Vector((0, -5, 0))
            bone["OriginVertex"] = origin

        editBonesArray = amt.edit_bones.values()
        for i, (name, origin, indices, parentIndex) in enumerate(groups):
            if parentIndex>=0:
                bone = editBonesArray[i]
                bone.parent = editBonesArray[parentIndex]

        bpy.ops.object.mode_set(mode='OBJECT')

        return amtobject

    def loadFile(self, context, filePath, scene) -> bpy.types.Object:
        self.validateAssetDirectory();
        
        log.debug("Loading file: %s" % filePath)

        with open(filePath, "rb") as f:
            data = f.read()
            if data[:3] == b"FTL":
                unpacked = data
                self.log.debug("Loaded %i unpacked bytes from file %s" % (len(data), filePath))
            else:
                unpacked = self.ioLib.unpack(data)
                with open(filePath + ".unpack", "wb") as f:
                    f.write(unpacked)
                    self.log.debug("Written unpacked ftl")
                self.log.debug("Loaded %i packed bytes from file %s" % (len(data), filePath))

        # create canonical id
        objPath = os.path.join(self.dataPath, "game/graph/obj3d/interactive")
        relPath = os.path.relpath(filePath, objPath)
        split = splitPath(relPath)
        canonicalId = split[:-1]

        object_id_string = "/".join(canonicalId);

        log.debug("Canonical ID: %s" % object_id_string)

        ftlData = self.ftlSerializer.read(unpacked)

        collection = bpy.data.collections.new(object_id_string)
        context.scene.collection.children.link(collection)

        bm = self.createBmesh(context, ftlData.verts, ftlData.faces)
        obj = self.createObject(context, bm, ftlData, canonicalId, collection)

        return obj

    # =============================================================

    def toFtlData(self) -> FtlData:

        objs = [o for o in bpy.data.objects if o.type == 'MESH' and not o.hide]

        if not objs:
            self.log.info("Nothing to export")

        obj = objs[0]

        canonicalId = obj.name.split("/")
        self.log.debug("Exporting Canonical Id: %s" % str(canonicalId))

        # obj.update_from_editmode()

        bm = bmesh.new()
        bm.from_object(obj, bpy.context.scene)
        bm.normal_update()

        arxFaceType = bm.faces.layers.int.get('arx_facetype')
        if not arxFaceType:
            raise ArxException("Mesh is missing arx specific data layer: " + 'arx_facetype')
        
        arxTransVal = bm.faces.layers.float.get('arx_transval')
        if not arxTransVal:
            raise ArxException("Mesh is missing arx specific data layer: " + 'arx_transval')
        
        if not (math.isclose(obj.location[0], 0.0) and math.isclose(obj.location[1], 0.0) and math.isclose(obj.location[2], 0.0)):
            raise ArxException("Object is moved, please apply the location to the vertex positions")
        
        if not (math.isclose(obj.rotation_euler[0], 0.0) and math.isclose(obj.rotation_euler[1], 0.0) and math.isclose(obj.rotation_euler[2], 0.0)):
            raise ArxException("Object is rotated, please apply the rotation to the vertex positions")
        
        if not (math.isclose(obj.scale[0], 1.0) and math.isclose(obj.scale[1], 1.0) and math.isclose(obj.scale[2], 1.0)):
            raise ArxException("Object is scaled, please apply the scale to the vertex positions")
        

        for f in bm.faces:
            if len(f.loops) > 3:
                raise ArxException("Face with more than 3 vertices found")

        for o in bpy.data.objects:
            for ms in o.material_slots:
                if not ms.material.name.endswith('-mat'):
                    raise ArxException("Material slot names must end with: " + '-mat')
                    
        
        # Validate child empties representing action points
        for child in bpy.data.objects:
            if child.type == 'EMPTY' and child.parent == obj:
                nameParts = child.name.split(".")
                name = nameParts[0].lower()
                print(nameParts)
                if name.startswith("hit_"):
                    pass
                elif name == "view_attach":
                    pass
                elif name == "primary_attach":
                    pass
                elif name == "left_attach":
                    pass
                elif name == "weapon_attach":
                    pass
                elif name == "secondary_attach":
                    pass
                elif name == "fire":
                    pass
                else:
                    self.log.warn("Unexpected child empty: {}".format(nameParts))
                    
        
        # TODO validate the mesh

        verts = []
        
        # First add all mesh vertices
        for v in bm.verts:
            vertexNormals = [sc.normal for sc in bm.verts if sc.co == v.co]
            normal = mathutils.Vector((0.0, 0.0, 0.0))
            for n in vertexNormals:
                normal += v.normal
            normal.normalize()
            verts.append(FtlVertex(v.co[:], normal))
        
        originVertexIndex = -1
        for index, vert in enumerate(verts):
            if vert.xyz == (0.0, 0.0, 0.0):
                self.log.info("Origin vertex found at index {}".format(index))
                originVertexIndex = index
        
        if originVertexIndex == -1:
            self.log.info("Origin vertex not found, adding new one")
            originVertexIndex = len(verts)
            verts.append(FtlVertex((0.0, 0.0, 0.0), (0.0, 0.0, 0.0)))
        
        metadata = FtlMetadata(name=obj.get('arx.ftl.name', ''), org=originVertexIndex)
        
        # Add action point vertices
        actions = []
        for o in bpy.data.objects:
            if o.type == 'EMPTY' and o.parent == obj:
                nameParts = o.name.split(".")
                name = nameParts[0]
                if o.parent_type == 'VERTEX':
                    actionVertexIndex = o.parent_vertices[0]
                    actions.append((name, actionVertexIndex))
                elif o.parent_type == 'OBJECT':
                    actionVertexIndex = len(verts)
                    verts.append(FtlVertex((o.location[0], o.location[1], o.location[2]), (0.0, 0.0, 0.0)))
                    actions.append((name, actionVertexIndex))
                else:
                    self.log.warn("Unhandled empty parent type {}".format(o.parent_type))

        uvData = bm.loops.layers.uv.verify()

        allXYZ = [v.xyz for v in verts]
        faces = []

        for f in bm.faces:
            vertexIndices = []
            vertexUvs = []
            vertexNormals = []
            for c in f.loops:
                vertexIndices.append(allXYZ.index(c.vert.co[:]))
                vertexUvs.append((c[uvData].uv[0], 1 - c[uvData].uv[1]))
                vertexNormals.append((c.vert.normal[0], c.vert.normal[1], c.vert.normal[2]))
            mat = f.material_index

            faceType = f[arxFaceType]
            transval = f[arxTransVal]

            faces.append(FtlFace(vids=vertexIndices, uvs=vertexUvs, texid=mat, facetype=faceType, transval=transval,
                                 normal=vertexNormals))

        grps = []
        sels = []

        dvert_lay = bm.verts.layers.deform.active

        for grp in obj.vertex_groups:
            v = []
            if dvert_lay is not None:
                for vert in bm.verts:
                    dvert = vert[dvert_lay]
                    if grp.index in dvert:
                        v.append(vert.index)

            s = grp.name.split(":")
            if s[0] == "grp":
                armature = bpy.data.armatures.get("/".join(canonicalId))
                if armature:
                    origin = armature.bones[grp.name]["OriginVertex"]
                else:
                    # TODO hardcoded use of mesh vertex at index 0
                    origin = 0
                
                grps.append((s[2], origin, v))
            else:
                sels.append((s[2], v))

        matNames = []
        for o in bpy.data.objects:
            for ms in o.material_slots:
                mat = ms.material
                # strip -mat suffix and decorate with bullshit
                matNames.append('GRAPH\\OBJ3D\\TEXTURES\\' + mat.name[:-4] + ".FOO")

        bm.free()

        return FtlData(
            metadata=metadata,
            verts=verts,
            faces=faces,
            mats=matNames,
            groups=grps,
            actions=actions,
            sels=sels
        )

    def saveFile(self, path):

        data = self.toFtlData()

        binData = self.ftlSerializer.write(data)

        with open(path, 'wb') as f:
            f.write(binData)

        self.log.debug("Written %i bytes to file %s" % (len(binData), path))
        

class ArxAnimationManager(object):
    def __init__(self):
        self.log = logging.getLogger('ArxAnimationManager')
        self.teaSerializer = TeaSerializer()
        
    def loadAnimation(self, path):
        data = self.teaSerializer.read(path)
        
        selectedObj = bpy.context.active_object

        #Walk up object tree to amature
        walkObj = selectedObj
        while not walkObj.name.endswith('-amt') and walkObj.parent:
            walkObj = walkObj.parent

        if not walkObj.name.endswith('-amt'):
            self.log.warning("Amature object nof found for: {}".format(obj.name))
            return

        armatureObj = walkObj
        obj = walkObj.children[0]

        bones = armatureObj.pose.bones
        
        bpy.context.scene.frame_set(1)
        for frame in data:
            bpy.context.scene.objects.active = obj
            
            if frame.translation:
                translation = frame.translation
                obj.location = (translation.x,translation.y,translation.z)
                obj.keyframe_insert(data_path='location')
                
            if frame.rotation:
                rotation = frame.rotation
                obj.rotation_quaternion = (rotation.w,rotation.x,rotation.y,rotation.z)
                obj.keyframe_insert(data_path='rotation_quaternion')

            #bpy.ops.anim.keyframe_insert(type='LocRotScale', confirm_success=False)
            
            bpy.context.scene.objects.active = armatureObj
            bpy.ops.object.mode_set(mode='POSE')

            if len(bones) != len(frame.groups):
                #raise InconsistentStateException("Bones in amature must match animation groups, existing {} new {}".format(len(bones), len(frame['groups'])))
                log.warning("Bones in amature must match animation groups, existing {} new {}".format(len(bones), len(frame.groups)))
                #break

            # This seems to be required to handle mismatched data
            maxBone = min(len(bones), len(frame.groups))

            for groupIndex in range(maxBone - 1, -1, -1): # group index = bone index
                group = frame.groups[groupIndex]
                bone = bones[groupIndex]
                location = Vector((group.translate.x,group.translate.y,group.translate.z))
                #self.log.info("moving bone to %s" % str(group.translate))
                #bone.location = location
                rotation = Quaternion((group.Quaternion.w,group.Quaternion.x,group.Quaternion.y,group.Quaternion.z))
                #self.log.info("rotating bone to %s" % str(rotation))
                bone.rotation_quaternion = rotation
                scale = Vector((group.zoom.x,group.zoom.y,group.zoom.z))
                #self.log.info("scaling bone to %s" % str(group.zoom))
                #bone.scale = scale
                bone.keyframe_insert(data_path="location")
                bone.keyframe_insert(data_path="rotation_quaternion")
                bone.keyframe_insert(data_path="scale")
                #bpy.ops.anim.keyframe_insert(type='LocRotScale', confirm_success=False)
                # FIXME translation and zoom are both 0,0,0
                # TODO keyframes of rotation are glitchy. probably cause im not doing it right. keyframe has to be set via bpy.ops.anim.keyframe_insert but i dont know how to select the bone.
                
            bpy.ops.object.mode_set(mode='OBJECT')
            
            bpy.context.scene.frame_set(bpy.context.scene.frame_current + frame.duration)
            #self.log.info("Loaded Frame")
        bpy.context.scene.frame_end = bpy.context.scene.frame_current


class ArxSceneManager(object):
    def __init__(self, ioLib, dataPath, arxFiles, objectManager):
        self.log = logging.getLogger('ArxSceneManager')
        self.dlfSerializer = DlfSerializer(ioLib)
        self.ftsSerializer = FtsSerializer(ioLib)
        self.llfSerializer = LlfSerializer(ioLib)
        self.dataPath = dataPath
        self.arxFiles = arxFiles
        self.objectManager = objectManager

    def importScene(self, context, sceneName, scene):
        self.log.info("Importing scene: %s" % sceneName)
        
        arxLevel = self.arxFiles.levels.levels[sceneName]
        
        if arxLevel.dlf is None:
            self.log.error("dlf file not found")
            return
        if arxLevel.fts is None:
            self.log.error("fts file not found")
            return
        if arxLevel.llf is None:
            self.log.error("llf file not found")
            return
        
        dlfData = self.dlfSerializer.readContainer(arxLevel.dlf)
        ftsData = self.ftsSerializer.read_fts_container(arxLevel.fts)
        llfData = self.llfSerializer.read(arxLevel.llf)

        # bpy.types.Material.Shader_Name = bpy.props.StringProperty(name='Group Name')

        # Create materials
        mappedMaterials = []
        idx = 0
        for tex in ftsData.textures:
            mappedMaterials.append((idx, tex.tc, createMaterial(self.dataPath, tex.fic.decode('iso-8859-1'))))
            idx += 1

        # Create mesh
        bm = self.AddSceneBackground(ftsData.cells, mappedMaterials)
        mesh = bpy.data.meshes.new(sceneName + "-mesh")
        bm.to_mesh(mesh)
        bm.free()

        # Create background object
        obj = bpy.data.objects.new(sceneName + "-background", mesh)
        scene.collection.objects.link(obj)
        # scn.objects.active = obj
        # obj.select = True

        # Create materials
        for idx, tcId, mat in mappedMaterials:
            obj.data.materials.append(mat)

        self.AddScenePathfinderAnchors(scene, ftsData.anchors)
        self.AddScenePortals(scene, ftsData)
        self.AddSceneLights(scene, llfData, ftsData.sceneOffset)
        self.AddSceneObjects(scene, dlfData, ftsData.sceneOffset)

    def AddSceneBackground(self, cells, mappedMaterials):
        bm = bmesh.new()
        uvLayer = bm.loops.layers.uv.verify()

        for z in cells:
            for cell in z:

                if cell is None:
                    continue

                for face in cell:

                    if face.type.POLY_QUAD:
                        to = 4
                    else:
                        to = 3

                    tempVerts = []
                    for i in range(to):
                        tempVerts.append(
                            ([face.v[i].ssx, face.v[i].sy, face.v[i].ssz], [face.v[i].stu, 1 - face.v[i].stv]))

                    # Switch the vertex order
                    if face.type.POLY_QUAD:
                        tempVerts[2], tempVerts[3] = tempVerts[3], tempVerts[2]

                    vertIdx = []
                    for i in tempVerts:
                        vertIdx.append(bm.verts.new(arx_pos_to_blender_for_model(i[0])))

                    bmFace = bm.faces.new(vertIdx)

                    if face.tex != 0:
                        matIdx = next((x for x in mappedMaterials if x[1] == face.tex), None)

                        if matIdx is not None:
                            bmFace.material_index = matIdx[0]
                        else:
                            self.log.info("Matrial id not found %i" % face.tex)

                    for i, loop in enumerate(bmFace.loops):
                        loop[uvLayer].uv = tempVerts[i][1]

        bm.verts.index_update()
        bm.edges.index_update()
        #bm.transform(correctionMatrix)
        return bm
    
    def AddScenePathfinderAnchors(self, scene, anchors):
        
        bm = bmesh.new()
        
        bVerts = []
        for anchor in anchors:
            bVerts.append(bm.verts.new(arx_pos_to_blender_for_model(anchor[0])))
        
        bm.verts.index_update()
        
        for i, anchor in enumerate(anchors):
            for edge in anchor[1]:
                #TODO this is a hack
                try:
                    bm.edges.new((bVerts[i], bVerts[edge]));
                except ValueError:
                    pass
        
        #bm.transform(correctionMatrix)
        mesh = bpy.data.meshes.new(scene.name + '-anchors-mesh')
        bm.to_mesh(mesh)
        bm.free()
        obj = bpy.data.objects.new(scene.name + '-anchors', mesh)
        # obj.draw_type = 'WIRE'
        # obj.show_x_ray = True
        scene.collection.objects.link(obj)

    def AddScenePortals(self, scene, data):
        portals_col = bpy.data.collections.new(scene.name + '-portals')
        scene.collection.children.link(portals_col)

        for portal in data.portals:
            bm = bmesh.new()

            tempVerts = []
            for vertex in portal.poly.v:
                pos = [vertex.pos.x, vertex.pos.y, vertex.pos.z]
                tempVerts.append(pos)

            # Switch the vertex order
            tempVerts[2], tempVerts[3] = tempVerts[3], tempVerts[2]

            bVerts = []
            for i in tempVerts:
                bVerts.append(bm.verts.new(arx_pos_to_blender_for_model(i)))

            bm.faces.new(bVerts)
            #bm.transform(correctionMatrix)
            mesh = bpy.data.meshes.new(scene.name + '-portal-mesh')
            bm.to_mesh(mesh)
            bm.free()
            obj = bpy.data.objects.new(scene.name + '-portal', mesh)
            obj.display_type = 'WIRE'
            obj.display.show_shadows = False
            # obj.show_x_ray = True
            # obj.hide = True
            #obj.parent_type = 'OBJECT'
            #obj.parent = groupObject
            portals_col.objects.link(obj)

    def AddSceneLights(self, scene, llfData, sceneOffset):
        lights_col = bpy.data.collections.new(scene.name + '-lights')
        scene.collection.children.link(lights_col)

        for index, light in enumerate(llfData.lights):
            light_name = scene.name + '-light_' + str(index).zfill(4)

            lampData = bpy.data.lights.new(name=light_name, type='POINT')
            lampData.color = (light.rgb.r, light.rgb.g, light.rgb.b)
            lampData.use_custom_distance = True
            lampData.cutoff_distance = light.fallend
            lampData.energy = light.intensity * 1000 # TODO this is a guessed factor

            obj = bpy.data.objects.new(name=light_name, object_data=lampData)
            lights_col.objects.link(obj)
            abs_loc = mathutils.Vector(sceneOffset) + mathutils.Vector([light.pos.x, light.pos.y, light.pos.z])
            obj.location = arx_pos_to_blender_for_model(abs_loc)


    def AddSceneObjects(self, scene, dlfData: DlfData, sceneOffset):
        entities_col = bpy.data.collections.new(scene.name + '-entities')
        scene.collection.children.link(entities_col)

        for e in dlfData.entities:
            
            legacyPath = e.name.decode('iso-8859-1').replace("\\", "/").lower().split('/')
            objectId = '/'.join(legacyPath[legacyPath.index('interactive') + 1 : -1])

            object_col = bpy.data.collections.get(objectId)

            if object_col is None:
                self.log.info("Object not found: [{}]".format(objectId))
                continue;

            entityId = objectId + "_" + str(e.ident).zfill(4)
            self.log.info("Creating entity [{}]".format(entityId))

            proxyObject = bpy.data.objects.new(name='e:' + entityId, object_data=None)
            entities_col.objects.link(proxyObject)
            proxyObject.dupli_type = 'COLLECTION'
            proxyObject.dupli_group = object_col

            pos = mathutils.Vector(sceneOffset) + mathutils.Vector([e.pos.x, e.pos.y, e.pos.z])
            proxyObject.location = arx_pos_to_blender_for_model(pos)

            # FIXME proper rotation conversion
            #proxyObject.rotation_mode = 'YXZ'
            proxyObject.rotation_euler = [math.radians(e.angle.a), math.radians(e.angle.g), math.radians(e.angle.b)]

class ArxAssetManager(object):
    def __init__(self, arxFiles, objectManager, sceneManager):
        self.log = logging.getLogger('ArxAssetManager')
        self.log.setLevel(logging.DEBUG)
        
        self.arxFiles = arxFiles
        self.objectManager = objectManager
        self.sceneManager = sceneManager

    def importAllModels(self, context):
        sortedModels = sorted(self.arxFiles.models.data.items())
        
        for i, (key, val) in enumerate(sortedModels):
            """
            sceneName = "m:" + '/'.join(key)
            
            scene = bpy.data.scenes.get(sceneName)
            
            if scene is None:
                self.log.info("Creating new scene [{}]".format(sceneName))
                scene = bpy.data.scenes.new(name=sceneName)
                scene.unit_settings.system = 'METRIC'
                scene.unit_settings.scale_length = 0.01
            else:
                self.log.info("Scene [{}] already exists".format(sceneName))
                continue
            """
            scene = None

            import_file = os.path.join(val.path, val.model)
            self.objectManager.loadFile(context, import_file, scene);


class ArxAddon(object):
    def __init__(self, dataPath, allowLibFallback):
        self.log = logging.getLogger('Arx Addon')
        
        try:
            ioLib = ArxIO()
        except Exception as e:
            if not allowLibFallback:
                raise e
            else:
                self.log.error("Failed to load native io library, using slow fallback. Exception: " + str(e));
                
                from .naivePkware import decompress_ftl
                class ArxIOFallback(object):
                    def __init__(self):
                        pass
                    def unpack(self, data):
                        return decompress_ftl(data)
                        
                ioLib = ArxIOFallback()
        
        self.arxFiles = ArxFiles(dataPath)
        self.arxFiles.updateAll()
        
        self.objectManager = ArxObjectManager(ioLib, dataPath)
        self.sceneManager = ArxSceneManager(ioLib, dataPath, self.arxFiles, self.objectManager)
        self.animationManager = ArxAnimationManager()
        self.assetManager = ArxAssetManager(self.arxFiles, self.objectManager, self.sceneManager)
