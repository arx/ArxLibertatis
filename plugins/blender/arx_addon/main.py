# Copyright 2015-2020 Arx Libertatis Team (see the AUTHORS file)
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

if "ArxAddon" in locals():
    importlib.reload(sys.modules["arx_addon.managers"])

if "ArxFacePanel" in locals():
    importlib.reload(sys.modules["arx_addon.meshEdit"])

from .arx_io_util import ArxException
from .managers import ArxAddon
from .meshEdit import ArxFacePanel, ArxMeshAddCustomProperties

import bpy
from bpy_extras.io_utils import ExportHelper, ImportHelper
from bpy.types import AddonPreferences
from bpy.props import BoolProperty, StringProperty
from .arx_ui_area import (
    arx_ui_area_register,
    arx_ui_area_unregister
)
from .arx_ui_model import (
    arx_ui_model_register,
    arx_ui_model_unregister
)
from .managers import arxAddonReload, getAddon

import logging
logging.basicConfig(level=logging.DEBUG)
log = logging.getLogger('ArxAddon')

class ArxAddonPreferences(AddonPreferences):
    bl_idname = __package__
    def reload(self, context):
        arxAddonReload()
    arxAssetPath: StringProperty(name="Arx assets root directory", subtype='DIR_PATH', update=reload)
    arxAllowLibFallback: BoolProperty(name="Allow use of fallback io library, only import ftl models, scenes are broken!", update=reload)
    def draw(self, context):
        layout = self.layout
        layout.prop(self, "arxAssetPath")
        layout.prop(self, "arxAllowLibFallback")

class ArxModelsPanel(bpy.types.Panel):
    bl_idname = "SCENE_PT_models_panel"
    bl_label = "Arx Libertatis Models"
    bl_space_type = 'PROPERTIES'
    bl_region_type = 'WINDOW'
    bl_context = "scene"
    def draw(self, context):
        layout = self.layout
        layout.operator("arx.operator_import_all_models")
        layout.operator("arx.operator_test_model_export")

class ArxLevelsPanel(bpy.types.Panel):
    bl_idname = "SCENE_PT_levels_panel"
    bl_label = "Arx Libertatis Levels"
    bl_space_type = 'PROPERTIES'
    bl_region_type = 'WINDOW'
    bl_context = "scene"
    def draw(self, context):
        layout = self.layout
        layout.operator("arx.operator_import_all_levels")
        layout.operator("arx.arx_area_list_reload")
        
        # Show area list
        area_list = context.window_manager.arx_areas_col
        if area_list:
            layout.template_list("SCENE_UL_arx_area_list", "", context.window_manager, "arx_areas_col", context.window_manager, "arx_areas_idx")
            row = layout.row()
            row.operator("arx.area_list_import_selected")
            
            # Export section
            layout.separator()
            layout.label(text="Export Area Data:")
            
            # Individual export buttons
            col = layout.column(align=True)
            col.operator("arx.area_export_fts", text="Export FTS (Geometry)")
            col.operator("arx.area_export_llf", text="Export LLF (Lighting)")
            col.operator("arx.area_export_dlf", text="Export DLF (Entities/Zones)")

            # NPCs cannot move without a navigation graph, and geometry built in
            # Blender arrives without one. Export generates it when the level has
            # none at all; this is how you rebuild one after reshaping a level.
            layout.separator()
            layout.operator("arx.generate_anchors", text="Generate Anchors (Navmesh)")
            
            # All export button
            layout.separator()
            layout.operator("arx.area_list_export_all", text="Export All Area Data")
            layout.operator("arx.view_face_attributes", text="View Face Attributes")

class ArxOperatorTestModelExport(bpy.types.Operator):
    bl_idname = "arx.operator_test_model_export"
    bl_label = "Test model export"
    bl_description = "Test exporting a model"
    def execute(self, context):
        try:
            getAddon(context).testModelExport()
            return {'FINISHED'}
        except ArxException as e:
            self.report({'ERROR'}, str(e))
            return {'CANCELLED'}

class ArxOperatorImportAllModels(bpy.types.Operator):
    bl_idname = "arx.operator_import_all_models"
    bl_label = "Import all models"
    bl_description = "Import all available models"
    def execute(self, context):
        try:
            getAddon(context).assetManager.importAllModels(context)
            return {'FINISHED'}
        except ArxException as e:
            self.report({'ERROR'}, str(e))
            return {'CANCELLED'}

class ImportFTL(bpy.types.Operator, ImportHelper):
    bl_idname = "arx.import_ftl"
    bl_label = 'Import FTL model'
    bl_description = 'Import Arx Fatalis Model (.ftl) file'
    bl_options = {'PRESET', 'UNDO'}
    filename_ext = ".ftl"
    check_extension = True
    filter_glob: StringProperty(default="*.ftl", options={'HIDDEN'})
    import_tweaks: BoolProperty(name="Import Tweaks", description="Import model tweaks", default=False)
    def execute(self, context):
        try:
            scene = context.scene
            getAddon(context).objectManager.loadFile(context, self.filepath, scene, self.import_tweaks)
            return {'FINISHED'}
        except ArxException as e:
            self.report({'ERROR'}, str(e))
            return {'CANCELLED'}

def menu_func_import_ftl(self, context):
    self.layout.operator(ImportFTL.bl_idname, text="Arx Fatalis Model (.ftl)")

class ExportFTL(bpy.types.Operator, ExportHelper):
    bl_idname = "arx.export_ftl"
    bl_label = 'Export FTL model'
    bl_description = 'Export Arx Fatalis Model (.ftl) file'
    bl_options = {'PRESET', 'UNDO'}
    filename_ext = ".ftl"
    check_extension = True
    filter_glob: StringProperty(default="*.ftl", options={'HIDDEN'})
    def execute(self, context):
        try:
            getAddon(context).objectManager.saveFile(self.filepath)
            return {'FINISHED'}
        except ArxException as e:
            self.report({'ERROR'}, str(e))
            return {'CANCELLED'}

def menu_func_export_ftl(self, context):
    self.layout.operator(ExportFTL.bl_idname, text="Arx Fatalis Model (.ftl)")

class ImportTea(bpy.types.Operator, ImportHelper):
    bl_idname = "arx.import_tea"
    bl_label = "Import Tea animation"
    bl_description = "Import Tea animation (.tea) file"
    bl_options = {'REGISTER', 'UNDO'}
    filename_ext = ".tea"
    check_extension = True
    filter_glob: StringProperty(default="*.tea", options={'HIDDEN'})
    def execute(self, context):
        try:
            getAddon(context).animationManager.loadAnimation(self.filepath)
            return {'FINISHED'}
        except ArxException as e:
            self.report({'ERROR'}, str(e))
            return {'CANCELLED'}

def menu_func_import_tea(self, context):
    self.layout.operator(ImportTea.bl_idname, text="Tea animation (.tea)")

class ExportTea(bpy.types.Operator, ExportHelper):
    bl_idname = "arx.export_tea"
    bl_label = 'Export Tea animation'
    bl_description = 'Export Tea animation (.tea) file'
    bl_options = {'PRESET', 'UNDO'}
    filename_ext = ".tea"
    check_extension = True
    filter_glob: StringProperty(default="*.tea", options={'HIDDEN'})
    
    # Animation export options
    action_name: StringProperty(
        name="Action Name",
        description="Name of the action to export (leave empty for active action)",
        default=""
    )
    frame_rate: bpy.props.FloatProperty(
        name="Frame Rate",
        description="Frame rate for the animation",
        default=24.0,
        min=1.0,
        max=120.0
    )
    scale_factor: bpy.props.FloatProperty(
        name="Scale Factor",
        description="Scale factor for positions",
        default=0.1,
        min=0.001,
        max=10.0
    )
    tea_version: bpy.props.EnumProperty(
        name="TEA Version",
        description="TEA file format version",
        items=[
            ('2014', "2014", "TEA version 2014"),
            ('2015', "2015", "TEA version 2015")
        ],
        default='2015'
    )
    
    def execute(self, context):
        try:
            # Get the active object
            obj = context.active_object
            if not obj or obj.type != 'MESH':
                self.report({'ERROR'}, "Please select a mesh object with an armature")
                return {'CANCELLED'}
            
            # Check if mesh has armature
            armature_obj = None
            for modifier in obj.modifiers:
                if modifier.type == 'ARMATURE' and modifier.object:
                    armature_obj = modifier.object
                    break
            
            if not armature_obj:
                self.report({'ERROR'}, "Selected mesh has no armature modifier")
                return {'CANCELLED'}
            
            # Get action name
            action_name = self.action_name if self.action_name else None
            
            # Export animation
            success = getAddon(context).animationManager.saveAnimation(
                self.filepath,
                action_name=action_name,
                frame_rate=self.frame_rate,
                scale_factor=self.scale_factor,
                version=int(self.tea_version)
            )
            
            if success:
                self.report({'INFO'}, f"Animation exported to {self.filepath}")
                return {'FINISHED'}
            else:
                self.report({'ERROR'}, "Failed to export animation")
                return {'CANCELLED'}
                
        except ArxException as e:
            self.report({'ERROR'}, str(e))
            return {'CANCELLED'}

def menu_func_export_tea(self, context):
    self.layout.operator(ExportTea.bl_idname, text="Tea animation (.tea)")

classes = (
    ArxAddonPreferences,
    ArxOperatorImportAllModels,
    ArxOperatorTestModelExport,
    ArxModelsPanel,
    ArxLevelsPanel,
    ArxMeshAddCustomProperties,
    ArxFacePanel,
    ImportFTL,
    ExportFTL,
    ImportTea,
    ExportTea
)

def register():
    log.debug("register")
    for cls in classes:
        bpy.utils.register_class(cls)
    arx_ui_area_register()
    bpy.types.TOPBAR_MT_file_import.append(menu_func_import_ftl)
    arx_ui_model_register()
    bpy.types.TOPBAR_MT_file_export.append(menu_func_export_ftl)
    bpy.types.TOPBAR_MT_file_import.append(menu_func_import_tea)
    bpy.types.TOPBAR_MT_file_export.append(menu_func_export_tea)

def unregister():
    log.debug("unregister")
    bpy.types.TOPBAR_MT_file_import.remove(menu_func_import_ftl)
    arx_ui_model_unregister()
    bpy.types.TOPBAR_MT_file_export.remove(menu_func_export_ftl)
    bpy.types.TOPBAR_MT_file_import.remove(menu_func_import_tea)
    bpy.types.TOPBAR_MT_file_export.remove(menu_func_export_tea)
    for cls in reversed(classes):
        bpy.utils.unregister_class(cls)
    arx_ui_area_unregister()
    arxAddonReload()
