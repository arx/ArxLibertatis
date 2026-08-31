#!/usr/bin/env python3

import sys
import os
sys.path.append('/home/burner/Desktop/ArxLibertatis/plugins/blender')

import bpy

# Test if the addon can be enabled and level import UI is available
def test_level_import_ui():
    # Check if addon is loaded
    addon_name = 'arx_addon'
    
    # Check if the ArxLevelsPanel is registered
    panels = [cls for cls in bpy.types.Panel.__subclasses__() 
              if hasattr(cls, 'bl_idname') and 'levels' in cls.bl_idname.lower()]
    
    print(f"Found level panels: {[p.bl_idname for p in panels]}")
    
    # Check if level import operators are registered
    operators = [cls for cls in bpy.types.Operator.__subclasses__() 
                 if hasattr(cls, 'bl_idname') and 'level' in cls.bl_idname.lower()]
    
    print(f"Found level operators: {[o.bl_idname for o in operators]}")
    
    # Check if area operators are registered
    area_operators = [cls for cls in bpy.types.Operator.__subclasses__() 
                      if hasattr(cls, 'bl_idname') and 'area' in cls.bl_idname.lower()]
    
    print(f"Found area operators: {[o.bl_idname for o in area_operators]}")

if __name__ == '__main__':
    test_level_import_ui()