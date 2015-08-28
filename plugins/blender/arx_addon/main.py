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
if "dataTea" in locals():
    imp.reload(dataTea)

if "lib" in locals():
    imp.reload(lib)

if "meshEdit" in locals():
    imp.reload(meshEdit)

import bpy

from bpy_extras.io_utils import (
    ExportHelper,
    ImportHelper,
    path_reference_mode
)

from bpy.types import (
    Operator,
    Panel,
    AddonPreferences
)

from bpy.props import (
    IntProperty,
    StringProperty,
    CollectionProperty
)

from .managers import ArxAddon
from .meshEdit import ArxFacePanel

import logging

logging.basicConfig(level = logging.DEBUG)
log = logging.getLogger('ArxAddon')

class ArxAddonPreferences(AddonPreferences):
    bl_idname = __package__

    arxAssetPath = StringProperty(name="Arx assets root directory", subtype='DIR_PATH')

    def draw(self, context):
        layout = self.layout
        layout.prop(self, "arxAssetPath")

def getAddon(context):
    addon_prefs = context.user_preferences.addons[__package__].preferences
    return ArxAddon(addon_prefs.arxAssetPath)

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

class ImportFTL(bpy.types.Operator, ImportHelper):
    '''Load an Arx Fatalis Model File'''
    bl_idname = "arx.import_ftl"
    bl_label = 'Import FTL model'
    bl_options = {'PRESET'}

    filename_ext = ".ftl"
    filter_glob = StringProperty(default="*.ftl", options={'HIDDEN'})
    check_extension = True
    path_mode = path_reference_mode

    def execute(self, context):
        getAddon(context).objectManager.loadFile(self.filepath)
        return {'FINISHED'}

def menu_func_import_ftl(self, context):
    self.layout.operator(ImportFTL.bl_idname, text="Arx Fatalis Model (.ftl)")

class ImportTea(bpy.types.Operator, ImportHelper):
    """Load a Tea animation file"""
    bl_idname = "arx.import_tea"
    bl_label = "Import Tea animation"
    bl_options = {'REGISTER', 'UNDO'}

    filename_ext = ".tea"
    filter_glob = StringProperty(default="*.tea", options={'HIDDEN'})
    check_extension = True
    path_mode = path_reference_mode

    def execute(self, context):
        from .dataTea import TeaSerializer

        serializer = TeaSerializer()
        data = serializer.read(self.filepath)

        # TODO implement skeleton loading here

        return {'FINISHED'}

def menu_func_import_tea(self, context):
    self.layout.operator(ImportTea.bl_idname, text="Tea animation (.tea)")

# ============= Registration

def register():
    log.info("ArxAddon register")
    bpy.utils.register_class(ArxAddonPreferences)

    bpy.utils.register_class(ArxScenesUpdateList)
    bpy.utils.register_class(ArxScenesImportSelected)
    bpy.utils.register_class(ArxScenesPanel)

    bpy.utils.register_class(ArxFacePanel)

    bpy.utils.register_class(ImportFTL)
    bpy.utils.register_class(ImportTea)
    bpy.types.INFO_MT_file_import.append(menu_func_import_ftl)
    bpy.types.INFO_MT_file_import.append(menu_func_import_tea)

def unregister():
    log.info("ArxAddon unregister")
    bpy.utils.unregister_class(ArxAddonPreferences)

    bpy.utils.unregister_class(ArxScenesUpdateList)
    bpy.utils.unregister_class(ArxScenesImportSelected)
    bpy.utils.unregister_class(ArxScenesPanel)

    bpy.utils.unregister_class(ArxFacePanel)

    bpy.utils.unregister_class(ImportFTL)
    bpy.utils.unregister_class(ImportTea)
    bpy.types.INFO_MT_file_import.remove(menu_func_import_ftl)
    bpy.types.INFO_MT_file_import.remove(menu_func_import_tea)

