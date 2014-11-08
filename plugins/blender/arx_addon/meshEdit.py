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

import bpy
import bmesh

from .dataCommon import PolyTypeFlag

class ArxFacePanel(bpy.types.Panel):
    bl_label = "Arx Face Properties"
    bl_space_type = 'VIEW_3D'
    bl_region_type = "UI"
    
    def draw(self, context):
        ob = bpy.context.object
        
        if ob is None:
            return
        
        if ob.type != 'MESH':
            layout.label(text="Not a mesh")
            return
        
        me = ob.data
        
        if me.is_editmode:
            # Gain direct access to the mesh
            bm = bmesh.from_edit_mesh(me)
        else:
            # Create a bmesh from mesh
            # (won't affect mesh, unless explicitly written back)
            bm = bmesh.new()
            bm.from_mesh(me)
        
        # Get active face
        face = bm.faces.active
        
        if face is None:
            return
        
        arxFaceType = bm.faces.layers.int.get('arx_facetype')
        arxTransVal = bm.faces.layers.float.get('arx_transval')
        
        layout = self.layout
        
        if arxFaceType is None or arxTransVal is None:
            layout.label(text="Not an arx mesh")
            return
        
        faceType = PolyTypeFlag()
        faceType.asUInt = face[arxFaceType]
        
        transval = face[arxTransVal]
        
        obj = bpy.context.active_object
        
        layout = self.layout
        
        layout.label(text="transval: " + str(transval))
        layout.label(text="POLY_NO_SHADOW: " + str(faceType.POLY_NO_SHADOW))
        layout.label(text="POLY_DOUBLESIDED: " + str(faceType.POLY_DOUBLESIDED))
        layout.label(text="POLY_TRANS: " + str(faceType.POLY_TRANS))
        layout.label(text="POLY_WATER: " + str(faceType.POLY_WATER))
        layout.label(text="POLY_GLOW: " + str(faceType.POLY_GLOW))
        layout.label(text="POLY_IGNORE: " + str(faceType.POLY_IGNORE))
        layout.label(text="POLY_QUAD: " + str(faceType.POLY_QUAD))
        layout.label(text="POLY_METAL: " + str(faceType.POLY_METAL))
        layout.label(text="POLY_HIDE: " + str(faceType.POLY_HIDE))
        layout.label(text="POLY_STONE: " + str(faceType.POLY_STONE))
        layout.label(text="POLY_WOOD: " + str(faceType.POLY_WOOD))
        layout.label(text="POLY_GRAVEL: " + str(faceType.POLY_GRAVEL))
        layout.label(text="POLY_EARTH: " + str(faceType.POLY_EARTH))
        layout.label(text="POLY_NOCOL: " + str(faceType.POLY_NOCOL))
        layout.label(text="POLY_LAVA: " + str(faceType.POLY_LAVA))
        layout.label(text="POLY_CLIMB: " + str(faceType.POLY_CLIMB))
        layout.label(text="POLY_FALL: " + str(faceType.POLY_FALL))
        layout.label(text="POLY_NOPATH: " + str(faceType.POLY_NOPATH))
        layout.label(text="POLY_NODRAW: " + str(faceType.POLY_NODRAW))
        layout.label(text="POLY_PRECISE_PATH: " + str(faceType.POLY_PRECISE_PATH))
        layout.label(text="POLY_LATE_MIP: " + str(faceType.POLY_LATE_MIP))