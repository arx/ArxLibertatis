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

if "ArxAddon" in locals():
    importlib.reload(sys.modules["arx_addon.managers"])

if "ArxFacePanel" in locals():
    importlib.reload(sys.modules["arx_addon.meshEdit"])

from .arx_io_util import ArxException
from .managers import ArxAddon
from .meshEdit import ArxFacePanel, ArxMeshAddCustomProperties

import bpy

from bpy_extras.io_utils import ExportHelper, ImportHelper, path_reference_mode

from bpy.types import AddonPreferences

from bpy.props import BoolProperty, StringProperty

import logging

from .arx_ui_area import (
    ArxOperatorImportLevel,
    ArxOperatorImportAllLevels,
    ArxAreaPanel
)

from .managers import (
    arxAddonReload,
    getAddon
)

logging.basicConfig(level=logging.DEBUG)
log = logging.getLogger('ArxAddon')

# ======================================================================================================================

class ArxAddonPreferences(bpy.types.AddonPreferences):
    bl_idname = __package__

    def reload(self, context):
        arxAddonReload()

    arxAssetPath: bpy.props.StringProperty(
        name="Arx assets root directory",
        subtype='DIR_PATH',
        update=reload
    )
    arxAllowLibFallback: bpy.props.BoolProperty(
        name="Allow use of fallback io library, only import ftl models, scenes are broken!",
        update=reload
    )

    def draw(self, context):
        layout = self.layout
        layout.prop(self, "arxAssetPath")
        layout.prop(self, "arxAllowLibFallback")

# ======================================================================================================================




"""
class ArxOperatorCreateLevelEditScreen(bpy.types.Operator):
    bl_idname = "arx.operator_create_level_edit_screen"
    bl_label = "Create level editing screen"
    
    def execute(self, context):
        name = "Arx Level Edit"
        
        if bpy.data.screens.get(name):
            self.report({'ERROR'}, "Screen already exists")
            return {'CANCELLED'}
        
        bpy.ops.screen.new()
        screen = bpy.context.window.screen
        screen.name = "Arx Level Edit"
        
        for a in bpy.context.screen.areas:
            if a.type == 'VIEW_3D':
                for s in a.spaces:
                    if s.type == 'VIEW_3D':
                        s.clip_end = 100000
                        s.show_relationship_lines = False
                        s.show_backface_culling = True
        
        return {'FINISHED'}
"""

# ======================================================================================================================

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

# ======================================================================================================================

class ArxOperatorTestModelExport(bpy.types.Operator):
    bl_idname = "arx.operator_test_model_export"
    bl_label = "Test model export"
    
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
    
    def execute(self, context):
        try:
            getAddon(context).assetManager.importAllModels(context)
            return {'FINISHED'}
        except ArxException as e:
            self.report({'ERROR'}, str(e))
            return {'CANCELLED'}



class ImportFTL(bpy.types.Operator, ImportHelper):
    '''Load an Arx Fatalis Model File'''
    bl_idname = "arx.import_ftl"
    bl_label = 'Import FTL model'
    bl_options = {'PRESET'}

    filename_ext = ".ftl"
    check_extension = True
    #path_mode = path_reference_mode

    filter_glob: StringProperty(
        default="*.ftl",
        options={'HIDDEN'}
    )

    def execute(self, context):
        try:
            scene = bpy.context.scene
            getAddon(context).objectManager.loadFile(context, self.filepath, scene);
            return {'FINISHED'}
        except ArxException as e:
            self.report({'ERROR'}, str(e))
            return {'CANCELLED'}


def menu_func_import_ftl(self, context):
    self.layout.operator(ImportFTL.bl_idname, text="Arx Fatalis Model (.ftl)")


class ExportFTL(bpy.types.Operator, ExportHelper):
    '''Save a raw uncompressed Arx Fatalis Model File'''
    bl_idname = "arx.export_ftl"
    bl_label = 'Export FTL model'
    bl_options = {'PRESET'}

    filename_ext = ".ftl"
    check_extension = True
    #path_mode = path_reference_mode

    filter_glob: StringProperty(
        default="*.ftl",
        options={'HIDDEN'}
    )

    def execute(self, context):
        try:
            getAddon(context).objectManager.saveFile(self.filepath)
            return {'FINISHED'}
        except ArxException as e:
            self.report({'ERROR'}, str(e))
            return {'CANCELLED'}


def menu_func_export_ftl(self, context):
    self.layout.operator(ExportFTL.bl_idname, text="Arx Fatalis Model (.ftl)")


# ======================================================================================================================

class ImportTea(bpy.types.Operator, ImportHelper):
    """Load a Tea animation file"""
    bl_idname = "arx.import_tea"
    bl_label = "Import Tea animation"
    bl_options = {'REGISTER', 'UNDO'}

    filename_ext = ".tea"
    check_extension = True
    #path_mode = path_reference_mode

    filter_glob: StringProperty(
        default="*.tea",
        options={'HIDDEN'}
    )

    def execute(self, context):
        getAddon(context).animationManager.loadAnimation(self.filepath)
        return {'FINISHED'}
    

def menu_func_import_tea(self, context):
    self.layout.operator(ImportTea.bl_idname, text="Tea animation (.tea)")


# ============= Registration

classes = (
    ArxAddonPreferences,
#
    ArxOperatorImportLevel,
    ArxOperatorImportAllLevels,
    ArxAreaPanel,
#
    ArxOperatorImportAllModels,
    ArxOperatorTestModelExport,
    ArxModelsPanel,
#
    ArxMeshAddCustomProperties,
    ArxFacePanel,
    ImportFTL,
    ExportFTL,
    ImportTea
)

def register():
    log.debug("register")

    for c in classes:
        bpy.utils.register_class(c)

    bpy.types.TOPBAR_MT_file_import.append(menu_func_import_ftl)
    bpy.types.TOPBAR_MT_file_export.append(menu_func_export_ftl)
    bpy.types.TOPBAR_MT_file_import.append(menu_func_import_tea)


def unregister():
    log.debug("unregister")

    bpy.types.TOPBAR_MT_file_import.remove(menu_func_import_ftl)
    bpy.types.TOPBAR_MT_file_export.remove(menu_func_export_ftl)
    bpy.types.TOPBAR_MT_file_import.remove(menu_func_import_tea)

    for c in classes:
        bpy.utils.unregister_class(c)

    arxAddonReload()
