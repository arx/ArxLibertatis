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

import logging

import bpy
import bmesh
from math import radians
from mathutils import Vector, Matrix

from .dataDlf import DlfSerializer, DlfData
from .dataFts import FtsSerializer
from .dataLlf import LlfSerializer

from .arx_io_material import createMaterial
from .arx_io_util import arx_pos_to_blender_for_model, ArxException

correctionMatrix = \
    Matrix.Rotation(radians(180), 4, 'Z') @ \
    Matrix.Rotation(radians(-90), 4, 'X')

class ArxSceneManager(object):
    def __init__(self, ioLib, dataPath, arxFiles, objectManager):
        self.log = logging.getLogger('ArxSceneManager')
        self.dlfSerializer = DlfSerializer(ioLib)
        self.ftsSerializer = FtsSerializer(ioLib)
        self.llfSerializer = LlfSerializer(ioLib)
        self.dataPath = dataPath
        self.arxFiles = arxFiles
        self.objectManager = objectManager

    def importScene(self, context, scene, area_id):
        self.log.info('Importing Area: {}'.format(area_id))
        
        area_files = self.arxFiles.levels.levels[area_id]
        
        if area_files.dlf is None:
            self.log.error("dlf file not found")
            return
        if area_files.fts is None:
            self.log.error("fts file not found")
            return
        if area_files.llf is None:
            self.log.error("llf file not found")
            return
        
        dlfData = self.dlfSerializer.readContainer(area_files.dlf)
        ftsData = self.ftsSerializer.read_fts_container(area_files.fts)
        llfData = self.llfSerializer.read(area_files.llf)

        # bpy.types.Material.Shader_Name = bpy.props.StringProperty(name='Group Name')

        # Create materials
        mappedMaterials = []
        idx = 0
        for tex in ftsData.textures:
            mappedMaterials.append((idx, tex.tc, createMaterial(self.dataPath, tex.fic.decode('iso-8859-1'))))
            idx += 1

        # Create mesh
        bm = self.AddSceneBackground(ftsData.cells, llfData.levelLighting, mappedMaterials)
        mesh = bpy.data.meshes.new(scene.name + "-mesh")
        bm.to_mesh(mesh)
        bm.free()

        # Create background object
        obj = bpy.data.objects.new(scene.name + "-background", mesh)
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
        self.add_scene_map_camera(scene)

    def AddSceneBackground(self, cells, levelLighting, mappedMaterials):
        bm = bmesh.new()
        uvLayer = bm.loops.layers.uv.verify()
        colorLayer = bm.loops.layers.color.new("light-color")

        


        vertexIndex = 0
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
                        pos = [face.v[i].ssx, face.v[i].sy, face.v[i].ssz]
                        uv = [face.v[i].stu, 1 - face.v[i].stv]
                        intCol = levelLighting[vertexIndex]
                        floatCol = (intCol.r / 255.0, intCol.g / 255.0, intCol.b / 255.0, intCol.a / 255.0)
                        tempVerts.append((pos, uv, floatCol))
                        vertexIndex += 1

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
                        loop[colorLayer] = tempVerts[i][2]

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
            abs_loc = Vector(sceneOffset) + Vector([light.pos.x, light.pos.y, light.pos.z])
            obj.location = arx_pos_to_blender_for_model(abs_loc)


    def AddSceneObjects(self, scene, dlfData: DlfData, sceneOffset):
        entities_col = bpy.data.collections.new(scene.name + '-entities')
        scene.collection.children.link(entities_col)

        for e in dlfData.entities:
            
            legacyPath = e.name.decode('iso-8859-1').replace("\\", "/").lower().split('/')
            objectId = '/'.join(legacyPath[legacyPath.index('interactive') + 1 : -1])

            entityId = objectId + "_" + str(e.ident).zfill(4)
            self.log.info("Creating entity [{}]".format(entityId))

            proxyObject = bpy.data.objects.new(name='e:' + entityId, object_data=None)
            entities_col.objects.link(proxyObject)

            object_col = bpy.data.collections.get(objectId)
            if object_col:
                proxyObject.instance_type = 'COLLECTION'
                proxyObject.instance_collection = object_col
            else:
                proxyObject.show_name = True
                proxyObject.empty_display_type = 'ARROWS'
                proxyObject.empty_display_size = 20 #cm
                #self.log.info("Object not found: [{}]".format(objectId))

            pos = Vector(sceneOffset) + Vector([e.pos.x, e.pos.y, e.pos.z])
            proxyObject.location = arx_pos_to_blender_for_model(pos)

            # FIXME proper rotation conversion
            #proxyObject.rotation_mode = 'YXZ'
            proxyObject.rotation_euler = [radians(e.angle.a), radians(e.angle.g), radians(e.angle.b)]

    def add_scene_map_camera(self, scene):
        """Grid size is 160x160m"""
        cam = bpy.data.cameras.new('Map Camera')
        cam.type = 'ORTHO'
        cam.ortho_scale = 16000
        cam.clip_start = 100 # 1m
        cam.clip_end = 20000 # 200m
        cam.show_name = True
        cam_obj = bpy.data.objects.new('Map Camera', cam)
        cam_obj.location = Vector((8000.0, 8000.0, 5000.0))
        scene.collection.objects.link(cam_obj)

        scene.render.engine = 'BLENDER_EEVEE'
        scene.render.resolution_x = 1000
        scene.render.resolution_y = 1000