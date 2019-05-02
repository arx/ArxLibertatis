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

# TODO this should be configurable
g_areaToLevel = {
    0:0, 8:0, 11:0, 12:0,
    1:1, 13:1, 14:1,
    2:2, 15:2,
    3:3, 16:3, 17:3,
    4:4, 18:4, 19:4,
    5:5, 21:5,
    6:6, 22:6,
    7:7, 23:7
}

def importArea(context, report, area_id):
    scene_name = "Area_" + str(area_id).zfill(2)
    scene = bpy.data.scenes.get(scene_name)
    if scene:
        report({'INFO'}, "Area Scene named [{}] already exists.".format(scene_name))
        return
    
    report({'INFO'}, "Creating new Area Scene [{}]".format(scene_name))
    scene = bpy.data.scenes.new(name=scene_name)
    scene.unit_settings.system = 'METRIC'
    scene.unit_settings.scale_length = 0.01
    
    getAddon(context).sceneManager.importScene(context, scene, area_id)


class CUSTOM_OT_arx_area_list_reload(Operator):
    bl_idname = "arx.arx_area_list_reload"
    bl_label = "Reload Area List"
    def invoke(self, context, event):
        area_list = context.window_manager.arx_areas_col
        area_list.clear()
        for area_id, value in getAddon(context).arxFiles.levels.levels.items():
            item = area_list.add()
            item.name = 'Area {}'.format(area_id)
            item.area_id = area_id
            item.level_id = g_areaToLevel.get(area_id, -1)
        return {"FINISHED"}

class ARX_area_properties(PropertyGroup):
    area_id: IntProperty(
        name="Arx Area ID",
        min=0
    )
    level_id: IntProperty(
        name="Arx Level ID",
        description="Levels are consist of areas",
        min=-1
    )

class SCENE_UL_arx_area_list(UIList):
    def draw_item(self, context, layout, data, item, icon, active_data, active_propname, index):
        split = layout.split(factor=0.3)
        split.label(text=item.name)
        split.label(text=str(item.area_id))
        split.label(text=str(item.level_id))

    def invoke(self, context, event):
        pass

class CUSTOM_OT_arx_area_list_import_selected(Operator):
    bl_idname = "arx.area_list_import_selected"
    bl_label = "Import Selected Area"
    def invoke(self, context, event):
        area_list = context.window_manager.arx_areas_col
        area = area_list[context.window_manager.arx_areas_idx]
        try:
            importArea(context, self.report, area.area_id)
        except ArxException as e:
            self.report({'ERROR'}, str(e))
            return {'CANCELLED'}
        return {'FINISHED'}

class ArxOperatorImportAllLevels(bpy.types.Operator):
    bl_idname = "arx.operator_import_all_levels"
    bl_label = "Import all levels"

    def execute(self, context):
        for area_id, value in getAddon(context).arxFiles.levels.levels.items():
            try:
                importArea(context, self.report, area_id)
            except ArxException as e:
                self.report({'ERROR'}, str(e))
                return {'CANCELLED'}
        return {'FINISHED'}

class ArxAreaPanel(Panel):
    bl_idname = "SCENE_PT_arx_areas"
    bl_label = "Arx Areas"
    bl_space_type = 'PROPERTIES'
    bl_region_type = 'WINDOW'
    bl_context = "scene"

    def draw(self, context):
        layout = self.layout
        layout.operator("arx.arx_area_list_reload")
        layout.template_list(
            listtype_name="SCENE_UL_arx_area_list",
            list_id="",
            dataptr=context.window_manager,
            propname="arx_areas_col",
            active_dataptr=context.window_manager,
            active_propname="arx_areas_idx"
        )
        layout.operator("arx.area_list_import_selected")
        layout.operator("arx.operator_import_all_levels")


classes = (
    CUSTOM_OT_arx_area_list_reload,
    ARX_area_properties,
    SCENE_UL_arx_area_list,
    CUSTOM_OT_arx_area_list_import_selected,
    ArxOperatorImportAllLevels,
    ArxAreaPanel,
)

def arx_ui_area_register():
    from bpy.utils import register_class
    for cls in classes:
        register_class(cls)

    bpy.types.WindowManager.arx_areas_col = CollectionProperty(type=ARX_area_properties)
    bpy.types.WindowManager.arx_areas_idx = IntProperty()

def arx_ui_area_unregister():
    from bpy.utils import unregister_class
    for cls in reversed(classes):
        unregister_class(cls)

    del bpy.types.WindowManager.arx_areas_col
    del bpy.types.WindowManager.arx_areas_idx