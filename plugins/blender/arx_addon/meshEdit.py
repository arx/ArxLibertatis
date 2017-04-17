# Copyright 2014-2017 Arx Libertatis Team (see the AUTHORS file)
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

import bmesh

import bpy

from .dataCommon import PolyTypeFlag

faceFlagNames = [
    ("POLY_NO_SHADOW", "NO_SHADOW"),
    ("POLY_DOUBLESIDED", "DOUBLESIDED"),
    ("POLY_TRANS", "TRANS"),
    ("POLY_WATER", "WATER"),
    ("POLY_GLOW", "GLOW"),
    ("POLY_IGNORE", "IGNORE"),
    ("POLY_QUAD", "QUAD"),
    ("POLY_METAL", "METAL"),
    ("POLY_HIDE", "HIDE"),
    ("POLY_STONE", "STONE"),
    ("POLY_WOOD", "WOOD"),
    ("POLY_GRAVEL", "GRAVEL"),
    ("POLY_EARTH", "EARTH"),
    ("POLY_NOCOL", "NOCOL"),
    ("POLY_LAVA", "LAVA"),
    ("POLY_CLIMB", "CLIMB"),
    ("POLY_FALL", "FALL"),
    ("POLY_NOPATH", "NOPATH"),
    ("POLY_NODRAW", "NODRAW"),
    ("POLY_PRECISE_PATH", "PRECISE_PATH"),
    ("POLY_LATE_MIP", "LATE_MIP")
]


class ArxMeshAddCustomProperties(bpy.types.Operator):
    bl_idname = "arx.mesh_add_custom_properties"
    bl_label = "Add custom properties to mesh"

    def execute(self, context):
        obj = bpy.context.object
        if obj is None:
            self.report({'ERROR'}, "No object selected")
            return {'CANCELLED'}
        if obj.type != 'MESH':
            self.report({'ERROR'}, "Selected object not a mesh")
            return {'CANCELLED'}
        
        if obj.data.is_editmode:
            bm = bmesh.from_edit_mesh(obj.data)
        else:
            bm = bmesh.new()
            bm.from_mesh(obj.data)
        
        if not bm.faces.layers.int.get('arx_facetype'):
            self.report({'INFO'}, "Adding missing " + 'arx_facetype' + " layer")
            bm.faces.layers.int.new('arx_facetype')
        
        if not bm.faces.layers.float.get('arx_transval'):
            self.report({'INFO'}, "Adding missing " + 'arx_transval' + " layer")
            bm.faces.layers.float.new('arx_transval')
        
        if bm.is_wrapped:
            bmesh.update_edit_mesh(obj.data, False, False)
        else:
            bm.to_mesh(obj.data)
            obj.data.update()
        
        return {'FINISHED'}

class ArxFacePanel(bpy.types.Panel):
    bl_label = "Arx Face Properties"
    bl_space_type = 'VIEW_3D'
    bl_region_type = "UI"

    bpy.types.Scene.ArxFaceFlags = bpy.props.BoolVectorProperty(name="ArxFaceFlags",
                                                                default=([False] * len(faceFlagNames)),
                                                                size=len(faceFlagNames))

    def draw(self, context):
        obj = bpy.context.object

        if obj is None:
            return
        
        layout = self.layout

        if obj.type != 'MESH':
            layout.label(text="Not a mesh")
            return

        if obj.data.is_editmode:
            bm = bmesh.from_edit_mesh(obj.data)
        else:
            bm = bmesh.new()
            bm.from_mesh(obj.data)

        arxFaceType = bm.faces.layers.int.get('arx_facetype')
        arxTransVal = bm.faces.layers.float.get('arx_transval')

        if arxFaceType is None or arxTransVal is None:
            layout.label(text="Not an arx mesh")
            layout.operator("arx.mesh_add_custom_properties");
            return

        face = bm.faces.active

        if face is None:
            return

        faceType = PolyTypeFlag()
        faceType.asUInt = face[arxFaceType]

        transval = face[arxTransVal]

        scene = context.scene

        layout.label(text="transval: " + str(transval))

        for i, (prop, text) in enumerate(faceFlagNames):
            a = getattr(faceType, prop)
            scene.ArxFaceFlags[i] = a

        row = layout.column(align=True)
        for i, (prop, text) in enumerate(faceFlagNames):
            row.prop(scene, "ArxFaceFlags", index=i, text=text, toggle=True)
