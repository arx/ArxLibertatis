# Copyright 2015 Arx Libertatis Team (see the AUTHORS file)
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
    importlib.reload(sys.modules["arx_addon.lib"])
    importlib.reload(sys.modules["arx_addon.dataDlf"])
    importlib.reload(sys.modules["arx_addon.dataFtl"])
    importlib.reload(sys.modules["arx_addon.dataFts"])
    importlib.reload(sys.modules["arx_addon.dataLlf"])
    # importlib.reload(sys.modules["arx_addon.dataTea"])

import bpy
import bmesh
import math
import mathutils
from mathutils import Vector,Quaternion

from .lib import ArxIO
from .dataDlf import DlfSerializer
from .dataFtl import FtlSerializer, FtlData, FtlMetadata, FtlVertex, FtlFace
from .dataFts import FtsSerializer
from .dataLlf import LlfSerializer
from .dataTea import TeaSerializer
from .common import *
from .files import *

correctionMatrix = \
    mathutils.Matrix.Rotation(math.radians(180), 4, 'Z') * \
    mathutils.Matrix.Rotation(math.radians(-90), 4, 'X') * \
    mathutils.Matrix.Scale(0.001, 4)


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

class InconsistentStateException(Exception):
    """Thrown if data supposed to be added to existing data does not match"""
    pass

import itertools

class ArxObjectManager(object):
    def __init__(self, ioLib, dataPath):
        self.log = logging.getLogger('ArxObjectManager')
        self.ioLib = ioLib
        self.dataPath = dataPath
        self.ftlSerializer = FtlSerializer()

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

    def createBmesh(self, vertData, faceData) -> bmesh.types.BMesh:

        facesToDrop = self.analyzeFaceData(faceData)

        bm = bmesh.new()

        arxFaceType = bm.faces.layers.int.new('arx_facetype')
        arxTransVal = bm.faces.layers.float.new('arx_transval')

        uvData = bm.loops.layers.uv.verify()
        for i, (position, normal) in enumerate(vertData):
            vertex = bm.verts.new(position)
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

    def createObject(self, bm, data, canonicalId) -> bpy.types.Object:

        mesh = bpy.data.meshes.new("/".join(canonicalId))
        bm.to_mesh(mesh)

        armatureObj = self.createArmature(canonicalId, bm, data.groups)

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
            bpy.context.scene.objects.link(action)
            action.parent = obj
            action.parent_type = 'VERTEX'
            action.parent_vertices = [vidx, 0, 0]  # last two are ignored
            action.show_x_ray = True
            action.show_name = True
            action.scale = [3, 3, 3]
            
        #armatureModifier = obj.modifiers.new(type='ARMATURE', name="Skeleton")
        #armatureModifier.object = armatureObj

        for toDeSel in bpy.context.scene.objects: #deselect all first just to be sure
            toDeSel.select = False

        bpy.context.scene.objects.link(obj)

        #FIXME properly bind bones to mesh via vertex groups
        obj.select = True
        armatureObj.select = True
        bpy.context.scene.objects.active = armatureObj #select both the mesh and armature and set armature active
        bpy.ops.object.parent_set(type='ARMATURE_AUTO') # ctrl+p and automatic weights
        bpy.context.scene.objects.active = obj
        armatureObj.select = False

        bpy.ops.object.mode_set(mode='EDIT')  # initialises UVmap correctly

        mesh.uv_textures.new()

        for m in data.mats:
            mat = createMaterial(self.dataPath, m)
            obj.data.materials.append(mat)
            
        bpy.ops.object.mode_set(mode='OBJECT')
        return obj

    def createArmature(self, canonicalId, bm, groups) -> bpy.types.Object:

        amtname = "/".join(canonicalId)
        origin = (0, 0, 0)

        # Create armature and object
        amt = bpy.data.armatures.new(amtname)
        amt.draw_type = 'WIRE'
        amtobject = bpy.data.objects.new(amtname + "-amt", amt)
        amtobject.show_x_ray = True
        amtobject.location = origin
        # ob.show_name = True

        # Link object to scene and make active
        bpy.context.scene.objects.link(amtobject)
        bpy.context.scene.objects.active = amtobject
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

    def addInstance(self, scene, classPath, ident):
        print("Creating new object type %s, id %i" % (classPath, ident))

        entityObject = 'game' + classPath + '.ftl'
        ftlFileName = os.path.join(self.dataPath, entityObject)
        print("Name: %s" % ftlFileName)
        return self.loadFile(ftlFileName)

    def loadFile(self, filePath) -> bpy.types.Object:
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
        log.debug("Canonical ID: %s" % str(canonicalId))

        ftlData = self.ftlSerializer.read(unpacked)

        bm = self.createBmesh(ftlData.verts, ftlData.faces)
        obj = self.createObject(bm, ftlData, canonicalId)

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

        metadata = FtlMetadata(name=obj.get('arx.ftl.name', ''), org=obj.get('arx.ftl.org', ''))

        bm = bmesh.new()
        bm.from_object(obj, bpy.context.scene)
        bm.normal_update()

        # TODO validate the mesh

        verts = []
        for v in bm.verts:
            vertexNormals = [sc.normal for sc in bm.verts if sc.co == v.co]
            normal = mathutils.Vector((0.0, 0.0, 0.0))
            for n in vertexNormals:
                normal += v.normal
            normal.normalize()
            verts.append(FtlVertex(v.co[:], normal))

        actions = []
        for o in bpy.data.objects:
            if o.type == 'EMPTY' and o.parent == obj:
                nameParts = o.name.split(".")
                vertex = o.parent_vertices[0]
                actions.append((nameParts[0], vertex))

        arxFaceType = bm.faces.layers.int.get('arx_facetype')
        arxTransVal = bm.faces.layers.float.get('arx_transval')

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
                origin = bpy.data.armatures["/".join(canonicalId)].bones[grp.name]["OriginVertex"]

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
    def __init__(self, ioLib, dataPath, objectManager):
        self.log = logging.getLogger('ArxSceneManager')
        self.dlfSerializer = DlfSerializer(ioLib)
        self.ftsSerializer = FtsSerializer(ioLib)
        self.llfSerializer = LlfSerializer(ioLib)
        self.dataPath = dataPath
        self.objectManager = objectManager

        self.arxFtsBaseDirectory = "game/graph/levels/"
        self.arxLlfBaseDirectory = "graph/levels/"

    def updateSceneList(self):
        baseDirectory = os.path.join(self.dataPath, self.arxFtsBaseDirectory)

        for sceneName in os.listdir(baseDirectory):
            if not sceneName in bpy.data.scenes:
                print("Adding Scene: %s" % sceneName)
                bpy.data.scenes.new(sceneName)

    def getSceneFiles(self, sceneName):
        result = {}
        result["fts"] = os.path.join(self.dataPath, self.arxFtsBaseDirectory, sceneName, "fast.fts")
        result["dlf"] = os.path.join(self.dataPath, self.arxLlfBaseDirectory, sceneName, sceneName + ".dlf")
        result["llf"] = os.path.join(self.dataPath, self.arxLlfBaseDirectory, sceneName, sceneName + ".llf")

        return result

    def importScene(self, sceneName):
        self.log.info("Importing scene: %s" % sceneName)

        files = self.getSceneFiles(sceneName)

        dlfData = self.dlfSerializer.readContainer(files["dlf"])
        ftsData = self.ftsSerializer.read_fts_container(files["fts"])
        llfData = self.llfSerializer.read(files["llf"])

        # bpy.types.Material.Shader_Name = bpy.props.StringProperty(name='Group Name')

        # Create materials
        mappedMaterials = []
        idx = 0
        for tex in ftsData.textures:
            mappedMaterials.append((idx, tex.tc, createMaterial(self.dataPath, tex.fic.decode('iso-8859-1'))))
            idx += 1

        scn = bpy.context.scene

        # Create mesh
        bm = self.AddSceneBackground(ftsData.cells, mappedMaterials)
        mesh = bpy.data.meshes.new(sceneName + "-mesh")
        bm.to_mesh(mesh)
        bm.free()

        # Create background object
        obj = bpy.data.objects.new(sceneName + "-background", mesh)
        scn.objects.link(obj)
        # scn.objects.active = obj
        # obj.select = True

        # Create materials
        for idx, tcId, mat in mappedMaterials:
            obj.data.materials.append(mat)

        self.AddScenePathfinderAnchors(scn, ftsData.anchors)
        self.AddScenePortals(scn, ftsData)
        self.AddSceneLights(scn, llfData, ftsData.sceneOffset)
        # TODO reenable object import
        #self.AddSceneObjects(scn, dlfData, ftsData.sceneOffset)

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
                        vertIdx.append(bm.verts.new(i[0]))

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
        bm.transform(correctionMatrix)
        return bm
    
    def AddScenePathfinderAnchors(self, scene, anchors):
        
        bm = bmesh.new()
        
        bVerts = []
        for anchor in anchors:
            bVerts.append(bm.verts.new(anchor[0]))
        
        bm.verts.index_update()
        
        for i, anchor in enumerate(anchors):
            for edge in anchor[1]:
                #TODO this is a hack
                try:
                    bm.edges.new((bVerts[i], bVerts[edge]));
                except ValueError:
                    pass
        
        bm.transform(correctionMatrix)
        mesh = bpy.data.meshes.new(scene.name + '-anchors-mesh')
        bm.to_mesh(mesh)
        bm.free()
        obj = bpy.data.objects.new(scene.name + '-anchors', mesh)
        obj.draw_type = 'WIRE'
        obj.show_x_ray = True
        scene.objects.link(obj)
        

    def AddScenePortals(self, scene, data):
        groupObject = bpy.data.objects.new(scene.name + '-portals', None)
        scene.objects.link(groupObject)

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
                bVerts.append(bm.verts.new(i))

            bm.faces.new(bVerts)
            bm.transform(correctionMatrix)
            mesh = bpy.data.meshes.new(scene.name + '-portal-mesh')
            bm.to_mesh(mesh)
            bm.free()
            obj = bpy.data.objects.new(scene.name + '-portal', mesh)
            obj.draw_type = 'WIRE'
            obj.show_x_ray = True
            # obj.hide = True
            obj.parent_type = 'OBJECT'
            obj.parent = groupObject
            scene.objects.link(obj)

    def AddSceneLights(self, scene, llfData, sceneOffset):
        groupObject = bpy.data.objects.new(scene.name + '-lights', None)
        scene.objects.link(groupObject)
        groupObject.location = correctionMatrix * mathutils.Vector(sceneOffset)

        for light in llfData.lights:
            lampData = bpy.data.lamps.new(name=scene.name + "-lamp-data", type='POINT')
            lampData.use_specular = False
            lampData.color = (light.rgb.r, light.rgb.g, light.rgb.b)
            lampData.use_sphere = True
            lampData.distance = light.fallend * 0.001  # TODO hardcoded scale
            lampData.energy = light.intensity
            obj = bpy.data.objects.new(name=scene.name + "-lamp", object_data=lampData)
            obj.location = correctionMatrix * mathutils.Vector([light.pos.x, light.pos.y, light.pos.z])
            obj.parent_type = 'OBJECT'
            obj.parent = groupObject

            scene.objects.link(obj)

    def AddSceneObjects(self, scene, dlfData, sceneOffset):

        for e in dlfData.entities:
            classPath = \
            os.path.splitext('/graph' + e.name.decode('iso-8859-1').replace("\\", "/").lower().split("graph", 1)[1])[0]
            ident = e.ident

            obj = self.objectManager.addInstance(scene, classPath, ident)
            obj.location = correctionMatrix * mathutils.Vector([e.pos.x, e.pos.y, e.pos.z])
            obj.location += correctionMatrix * mathutils.Vector(sceneOffset)

            # FIXME proper rotation conversion
            obj.rotation_mode = 'YXZ'
            obj.rotation_euler = [math.radians(e.angle.a-90), math.radians(e.angle.g+90), math.radians(e.angle.b)]
            obj.scale = [0.001,0.001,0.001] # TODO hardcoded scale


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
            
        self.objectManager = ArxObjectManager(ioLib, dataPath)
        self.sceneManager = ArxSceneManager(ioLib, dataPath, self.objectManager)
        self.animationManager = ArxAnimationManager()
