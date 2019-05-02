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

import bpy

from bpy.props import (
    IntProperty,
    BoolProperty,
    StringProperty,
    CollectionProperty,
    PointerProperty
)

from bpy.types import (
    Operator,
    Panel,
    PropertyGroup,
    UIList
)

from .arx_io_util import (
    ArxException
)

from .managers import getAddon


class ArxOperatorImportLevel(Operator):
    bl_idname = "arx.operator_import_level"
    bl_label = "Import level"
    
    levelName: bpy.props.StringProperty()
    
    def invoke(self, context, event):
        wm = context.window_manager
        return wm.invoke_props_dialog(self)
    
    def execute(self, context):
        levelId = int(self.levelName[5:]) # strip of level prefix
        sceneName = "level-" + str(levelId).zfill(3)
        
        scene = bpy.data.scenes.get(sceneName)
        if scene:
            self.report({'INFO'}, "Scene [{}] already exists".format(sceneName))
            return {'CANCELLED'}
        
        self.report({'INFO'}, "Creating new scene [{}]".format(sceneName))
        scene = bpy.data.scenes.new(name=sceneName)
        scene.unit_settings.system = 'METRIC'
        scene.unit_settings.scale_length = 0.01
        
        try:
            getAddon(context).sceneManager.importScene(context, self.levelName, scene)
            return {'FINISHED'}
        except ArxException as e:
            self.report({'ERROR'}, str(e))
            return {'CANCELLED'}

class ArxOperatorImportAllLevels(bpy.types.Operator):
    bl_idname = "arx.operator_import_all_levels"
    bl_label = "Import all levels"

    def execute(self, context):
        for levelName, value in getAddon(context).arxFiles.levels.levels.items():
            bpy.ops.arx.operator_import_level('EXEC_DEFAULT', levelName=levelName)
        
        return {'FINISHED'}

class ArxAreaPanel(Panel):
    bl_idname = "SCENE_PT_arx_areas"
    bl_label = "Arx Libertatis Scenes"
    bl_space_type = 'PROPERTIES'
    bl_region_type = 'WINDOW'
    bl_context = "scene"

    def draw(self, context):
        layout = self.layout
        layout.operator("arx.operator_import_level")
        layout.operator("arx.operator_import_all_levels")