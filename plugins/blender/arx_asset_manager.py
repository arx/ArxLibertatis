bl_info = {
    "name": "Arx Asset Manager",
    "author": "Eli2",
    "version": (1, 0),
    "blender": (2, 72, 0),
    "location": "",
    "description": "Addon for managing Arx Libertatis assets",
    "warning": "",
    "wiki_url": "",
    "tracker_url": "",
    "category": "Object"
}

import os
import bpy
import math
import mathutils

class ArxCreateOverviewScene(bpy.types.Operator):
    bl_idname = "arx.create_overview_scene"
    bl_label = "Import all Objects"
    
    overviewSceneName = "All Arx Assets"
    overviewCameraName = "Overview Camera"
    
    def execute(self, context):
        if not self.overviewSceneName in bpy.data.scenes:
            print("Creating new overview scene")
            bpy.ops.scene.new(type='NEW')
            bpy.context.scene.name = self.overviewSceneName
        
        bpy.context.screen.scene = bpy.data.scenes[self.overviewSceneName]
        
        # Delete everything
        scn = bpy.context.scene
        for obj in scn.objects:
            scn.objects.unlink(obj)
        
        assetsRootDirectory = str(bpy.context.scene.ArxAssetPath)
        print(assetsRootDirectory)
        for root, dirs, files in os.walk(assetsRootDirectory):
            for name in files:
                fileName, extension = os.path.splitext(name)
                if extension == ".ftl":
                    fullPath = str(root + "/" + name)
                    try:
                        bpy.ops.import_scene.ftl(filepath=fullPath, assetsRootDirectory=assetsRootDirectory, reorient=True)
                        obj = bpy.context.active_object
                        obj.name = fullPath
                        #obj.show_bounds = True
                        obj.show_axis = True
                    except:
                        print("Failed to import: %s" % fullPath)
        
        return{'FINISHED'}
    
class ArxLayoutOverviewScene(bpy.types.Operator):
    bl_idname = "arx.layout_overview_scene"
    bl_label = "Update Layout"
    
    def getSortValue(ob):
        minx = ob.bound_box[0][0] * ob.scale.x
        maxx = ob.bound_box[4][0] * ob.scale.x
        miny = ob.bound_box[0][1] * ob.scale.y
        maxy = ob.bound_box[2][1] * ob.scale.y
        minz = ob.bound_box[0][2] * ob.scale.z
        maxz = ob.bound_box[1][2] * ob.scale.z
        
        absx = abs(minx) + abs(maxx)
        absy = abs(miny) + abs(maxy)
        absz = abs(minz) + abs(maxz)
        
        return absx + absy + absz
    
    def execute(self, context):
        for ob in bpy.context.scene.objects:
            ob.location = [0, 0, 0]
        
        # Sort objects by size
        objs = sorted(bpy.context.scene.objects, key = lambda ob: getSortValue(ob))
        
        # Sort by name
        #objs = sorted(bpy.context.scene.objects, key=lambda ob: ob.name)
        
        posx = 0.0
        posy = 0
        rowMiny = 0
        rowMaxy = 0
        
        lastRowSize = [0, 0]
        
        for ob in objs:
            minx = ob.bound_box[0][0] * ob.scale.x
            maxx = ob.bound_box[4][0] * ob.scale.x
            miny = ob.bound_box[0][1] * ob.scale.y
            maxy = ob.bound_box[2][1] * ob.scale.y
            
            posx -= minx
            ob.location[0] = posx
            ob.location[1] = posy
            ob.location[2] = 0
            posx += maxx + 0.5
            
            rowMiny = min(rowMiny, miny)
            rowMaxy = max(rowMaxy, maxy)
            
            if posx > 500:
                posx = 0
                posy += rowMaxy + abs(rowMiny)
                
                lastRowSize = [rowMiny]
                
                rowMiny = 0
                rowMaxy = 0
                
        return{'FINISHED'}

class ArxPanel(bpy.types.Panel):
    bl_idname = "arx.Panel"
    bl_label = "Arx Libertatis Assets"
    bl_space_type = 'PROPERTIES'
    bl_region_type = 'WINDOW'
    bl_context = "scene"
    
    bpy.types.Scene.ArxAssetPath = bpy.props.StringProperty(name="Assets root directory", subtype='DIR_PATH')
    
    def draw(self, context):
        layout = self.layout
        scn = context.scene
        layout.operator("arx.layout_overview_scene")
        layout.separator()
        layout.prop(scn, 'ArxAssetPath')
        layout.operator("arx.create_overview_scene")

def register():
    bpy.utils.register_class(ArxCreateOverviewScene)
    bpy.utils.register_class(ArxLayoutOverviewScene)
    bpy.utils.register_class(ArxPanel)

def unregister():
    bpy.utils.unregister_class(ArxCreateOverviewScene)
    bpy.utils.unregister_class(ArxLayoutOverviewScene)
    bpy.utils.unregister_class(ArxPanel)
