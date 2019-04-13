# Copyright 2019 Arx Libertatis Team (see the AUTHORS file)
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

import os
import itertools
import logging
import math

import bpy
import bmesh
from mathutils import Vector

from .arx_io_util import arx_pos_to_blender_for_model, ArxException
from .arx_io_material import createMaterial

from .files import splitPath

from .dataFtl import FtlSerializer, FtlData, FtlMetadata, FtlVertex, FtlFace

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
            self.log.debug("Dropping faces: " + str(facesToDrop))

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
        
        self.log.debug("Loading file: %s" % filePath)

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

        # build the collection tree
        parent_collection = context.scene.collection
        idLen = len(canonicalId)
        for i in range(1, idLen + 1):
            prefix_str = "/".join(canonicalId[:i])
            if prefix_str in bpy.data.collections:
                parent_collection = bpy.data.collections[prefix_str]
            else:
                col = bpy.data.collections.new(prefix_str)
                parent_collection.children.link(col)
                parent_collection = col


        object_id_string = "/".join(canonicalId);

        self.log.debug("Canonical ID: %s" % object_id_string)

        ftlData = self.ftlSerializer.read(unpacked)

        #collection = bpy.data.collections.new(object_id_string)
        #context.scene.collection.children.link(collection)

        bm = self.createBmesh(context, ftlData.verts, ftlData.faces)
        obj = self.createObject(context, bm, ftlData, canonicalId, parent_collection)

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
            normal = Vector((0.0, 0.0, 0.0))
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