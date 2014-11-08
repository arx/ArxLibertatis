# Copyright 2014 Arx Libertatis Team (see the AUTHORS file)
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

bl_info = {
    "name": "Arx Libertatis Addon",
    "author": "Arx Libertatis Team",
    "version": (0, 0, 1),
    "blender": (2, 72, 0),
    "location": "",
    "description": "Addon for managing Arx Libertatis assets",
    "warning": "",
    "wiki_url": "",
    "tracker_url": "https://bugs.arx-libertatis.org/",
    "category": "Import-Export"
}

import imp


if "common" in locals():
    imp.reload(common)
if "dataCommon" in locals():
    imp.reload(dataCommon)

if "dataDlf" in locals():
    imp.reload(dataDlf)
if "dataFtl" in locals():
    imp.reload(dataFtl)
if "dataFts" in locals():
    imp.reload(dataFts)
if "dataLlf" in locals():
    imp.reload(dataLlf)

if "lib" in locals():
    imp.reload(lib)

if "meshEdit" in locals():
    imp.reload(meshEdit)

import os

import bpy
from bpy.types import AddonPreferences
from bpy.props import (
    IntProperty,
    StringProperty,
    CollectionProperty
)

import math
import mathutils
import bmesh

from .lib import ArxIO
from .dataDlf import DlfSerializer
from .dataFtl import FtlSerializer
from .dataFts import FtsSerializer
from .dataLlf import LlfSerializer
from .meshEdit import ArxFacePanel
from .common import *

import logging

logging.basicConfig(level = logging.DEBUG)
log = logging.getLogger('arx addon')

correctionMatrix = \
    mathutils.Matrix.Rotation(math.radians(180), 4, 'Z') *\
    mathutils.Matrix.Rotation(math.radians(-90), 4, 'X') *\
    mathutils.Matrix.Scale(0.001, 4)


class ArxObjectManager(object):
    
    def __init__(self, ioLib, dataPath):
        self.log = logging.getLogger('ArxObjectManager')
        self.dataPath = dataPath
        self.ftlSerializer = FtlSerializer(ioLib)
    
    def strip_wires(self, bm):
        [bm.verts.remove(v) for v in bm.verts if v.is_wire]
        [bm.verts.remove(v) for v in bm.verts if not v.link_faces[:]]
        [bm.edges.remove(e) for e in bm.edges if not e.link_faces[:]]
        [bm.faces.remove(f) for f in bm.faces if len(f.edges) < 3]
        for seq in [bm.verts, bm.faces, bm.edges]: seq.index_update()
        return bm
    
    def build_initial_bmesh(self, vertData, faceData):
        bm = bmesh.new()
        
        arxFaceType = bm.faces.layers.int.new('arx_facetype')
        arxTransVal = bm.faces.layers.float.new('arx_transval')
        
        uvData = bm.loops.layers.uv.verify()
        for xyz in vertData:
            bm.verts.new(xyz)
        
        bm.verts.index_update()
        
        for (vertIndexes, uvs, mat, faceType, trans) in faceData:
            faceVerts = [bm.verts[v] for v in vertIndexes]
            evDict = {}
            try:
                f = bm.faces.new(faceVerts)
            except ValueError:
                extraVerts = []
                for v in faceVerts:
                    ev = bm.verts.new(v.co)
                    bm.verts.index_update()
                    extraVerts.append(ev)
                    evDict[v] = ev
                f = bm.faces.new(extraVerts)
            
            if mat >= 0:
                f.material_index = mat
            
            f[arxFaceType] = faceType
            f[arxTransVal] = trans
                
            bm.faces.index_update()
            for c in f.loops:
                u, v = next(uvs)
                c[uvData].uv = u, 1.0 - v
        
        bm.edges.index_update()
        bm.transform(correctionMatrix)
        return (bm,evDict)

    def assign_groups(self, bm, groups, evDict):
        weightData = bm.verts.layers.deform.verify()
        gi = 0
        for (whatever,indexes) in groups:
            extra = []
            for i in indexes:
                if i in evDict.keys():
                    extra.extend(evDict[i])
            indexes.extend(extra)
            for vi in indexes:
                bm.verts[vi][weightData][gi] = 1.0
            gi += 1
        return bm
    
    def make_object(self, bm, data, evDict, name, assetsRootDirectory):
        groups = data['groups']
        groups.extend([("sel:"+n,i) for (n,i) in data['sels']])
        mesh = bpy.data.meshes.new(name)
        bm = self.assign_groups(bm, groups, evDict)
        bm = self.strip_wires(bm)
        bm.to_mesh(mesh)
        bm.free()
        if bpy.context.active_object:
            bpy.ops.object.mode_set(mode='OBJECT')
        obj = bpy.data.objects.new(name=mesh.name, object_data=mesh)
        for (name,whatever) in groups:
            obj.vertex_groups.new(name=name)
        bpy.context.scene.objects.link(obj)
        bpy.context.scene.objects.active = obj
        bpy.ops.object.mode_set(mode='EDIT') # initialises UVmap correctly
        mesh.uv_textures.new()
        for m in data['mats']:
            mat = createMaterial(assetsRootDirectory, m)
            obj.data.materials.append(mat)
        
        for (name,coords) in data['actions']:
            action = bpy.data.objects.new(name, None)
            bpy.context.scene.objects.link(action)
            action.parent = obj
            action.location = correctionMatrix * mathutils.Vector(coords)
    
        return obj
    
    def addInstance(self, scene, classPath, ident):
        print("Creating new object type %s, id %i" % (classPath, ident))
    
        entityObject = 'game' + classPath + '.ftl'
        ftlFileName = os.path.join(self.dataPath, entityObject)
        print("Name: %s" % ftlFileName)
    
        ftlData = self.ftlSerializer.readFile(ftlFileName)
        
        bm, evDict = self.build_initial_bmesh(ftlData['verts'], ftlData['faces'])
        
        entityName = "{0:s}_{1:04d}".format(classPath, ident)
        
        msg = "File \"" + ftlFileName + "\" loaded successfully."
        obj = self.make_object(bm, ftlData, evDict, entityName, self.dataPath)
        return obj


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
        log.info("Importing scene: %s" % sceneName)
        
        files = self.getSceneFiles(sceneName)
        
        dlfData = self.dlfSerializer.readContainer(files["dlf"])
        ftsData = self.ftsSerializer.read_fts_container(files["fts"])
        llfData = self.llfSerializer.read(files["llf"])
        
        #bpy.types.Material.Shader_Name = bpy.props.StringProperty(name='Group Name')
        
        # Create materials
        mappedMaterials = []
        idx = 0
        for tex in ftsData['textures']:
            mappedMaterials.append( (idx, tex.tc, createMaterial(self.dataPath, tex.fic.decode('iso-8859-1'))) )
            idx += 1
        
        scn = bpy.context.scene
        
        # Create mesh
        bm = self.AddSceneBackground(ftsData["cells"], mappedMaterials)
        mesh = bpy.data.meshes.new(sceneName + "-mesh")
        bm.to_mesh(mesh)
        bm.free()
        
        # Create background object
        obj = bpy.data.objects.new(sceneName + "-background", mesh)
        scn.objects.link(obj)
        #scn.objects.active = obj
        #obj.select = True
        
        # Create materials
        for idx, tcId, mat in mappedMaterials:
            obj.data.materials.append(mat)
        
        self.AddScenePortals(scn, ftsData)
        self.AddSceneLights(scn, llfData, ftsData["sceneOffset"])
        self.AddSceneObjects(scn, dlfData, ftsData["sceneOffset"])


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
                        tempVerts.append( ([face.v[i].ssx, face.v[i].sy, face.v[i].ssz], [face.v[i].stu, 1 - face.v[i].stv]) )
                    
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
                            log.info("Matrial id not found %i" % face.tex)
                
                    for i, loop in enumerate(bmFace.loops):
                        loop[uvLayer].uv = tempVerts[i][1]
        
        bm.verts.index_update()
        bm.edges.index_update()
        bm.transform(correctionMatrix)
        return bm

    def AddScenePortals(self, scene, data):
        groupObject = bpy.data.objects.new(scene.name + '-portals', None)
        scene.objects.link(groupObject)
        
        for portal in data["portals"]:
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
            #obj.hide = True
            obj.parent_type = 'OBJECT'
            obj.parent = groupObject
            scene.objects.link(obj)

    def AddSceneLights(self, scene, llfData, sceneOffset):
        groupObject = bpy.data.objects.new(scene.name + '-lights', None)
        scene.objects.link(groupObject)
        groupObject.location = correctionMatrix * mathutils.Vector(sceneOffset)
        
        for light in llfData["lights"]:
            lampData = bpy.data.lamps.new(name = scene.name + "-lamp-data", type = 'POINT')
            lampData.use_specular = False
            lampData.color = (light.rgb.r, light.rgb.g, light.rgb.b)
            lampData.use_sphere = True
            lampData.distance = light.fallend * 0.001 # TODO hardcoded scale
            lampData.energy = light.intensity
            obj = bpy.data.objects.new(name = scene.name + "-lamp", object_data = lampData)
            obj.location = correctionMatrix * mathutils.Vector([light.pos.x, light.pos.y, light.pos.z])
            obj.parent_type = 'OBJECT'
            obj.parent = groupObject
            
            scene.objects.link(obj)

    def AddSceneObjects(self, scene, dlfData, sceneOffset):
        
        for e in dlfData['entities']:
            classPath = os.path.splitext('/graph' + e.name.decode('iso-8859-1').replace("\\", "/").lower().split("graph", 1)[1])[0]
            ident = e.ident
            
            obj = self.objectManager.addInstance(scene, classPath, ident)
            obj.location = correctionMatrix * mathutils.Vector([e.pos.x, e.pos.y, e.pos.z])
            obj.location += correctionMatrix * mathutils.Vector(sceneOffset)
            
            # FIXME proper rotation conversion
            obj.rotation_mode = 'YXZ'
            obj.rotation_euler = [math.radians(e.angle.a), math.radians(e.angle.g), math.radians(e.angle.b)]


class ArxAddon(object):
    def __init__(self, dataPath):
        ioLib = ArxIO()
        self.objectManager = ArxObjectManager(ioLib, dataPath)
        self.sceneManager = ArxSceneManager(ioLib, dataPath, self.objectManager)

# ============= GUI

def getAddon(context):
    addon_prefs = context.user_preferences.addons[__name__].preferences
    return ArxAddon(addon_prefs.arxAssetPath)

class ArxAddonPreferences(AddonPreferences):
    bl_idname = __name__

    arxAssetPath = StringProperty(name="Arx assets root directory", subtype='DIR_PATH')

    def draw(self, context):
        layout = self.layout
        layout.prop(self, "arxAssetPath")


class ArxScenesUpdateList(bpy.types.Operator):
    bl_idname = "arx.update_scene_list"
    bl_label = "Update scene list"
    
    def execute(self, context):
        getAddon(context).sceneManager.updateSceneList()
        
        return{'FINISHED'}

class ArxScenesImportSelected(bpy.types.Operator):
    bl_idname = "arx.import_selected_scene"
    bl_label = "Import scene"
    
    def execute(self, context):
        sceneName = bpy.context.screen.scene.name
        getAddon(context).sceneManager.importScene(sceneName)
        
        return{'FINISHED'}

class ArxScenesPanel(bpy.types.Panel):
    bl_idname = "arx.scene.Panel"
    bl_label = "Arx Libertatis Scenes"
    bl_space_type = 'PROPERTIES'
    bl_region_type = 'WINDOW'
    bl_context = "scene"
    
    def draw(self, context):
        layout = self.layout
        layout.operator("arx.update_scene_list")
        layout.operator("arx.import_selected_scene")


def register():
    bpy.utils.register_class(ArxAddonPreferences)
    
    bpy.utils.register_class(ArxScenesUpdateList)
    bpy.utils.register_class(ArxScenesImportSelected)
    bpy.utils.register_class(ArxScenesPanel)
    
    bpy.utils.register_class(ArxFacePanel)

def unregister():
    bpy.utils.unregister_class(ArxAddonPreferences)
    
    bpy.utils.unregister_class(ArxScenesUpdateList)
    bpy.utils.unregister_class(ArxScenesImportSelected)
    bpy.utils.unregister_class(ArxScenesPanel)
    
    bpy.utils.unregister_class(ArxFacePanel)

if __name__ == "__main__":
    register()