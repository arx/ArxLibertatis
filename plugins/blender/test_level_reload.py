#!/usr/bin/env python3

import sys
import os
sys.path.append('/home/burner/Desktop/ArxLibertatis/plugins/blender')

import bpy
from arx_addon.managers import getAddon

# Test if the area list reload works
def test_area_list_reload():
    try:
        # Get addon context
        addon = getAddon(bpy.context)
        
        # Check if levels exist
        print(f"Levels object: {addon.arxFiles.levels}")
        print(f"Levels.levels: {addon.arxFiles.levels.levels}")
        print(f"Available levels: {list(addon.arxFiles.levels.levels.keys())}")
        
        # Try to reload area list
        area_list = bpy.context.window_manager.arx_areas_col
        area_list.clear()
        
        for area_id, value in addon.arxFiles.levels.levels.items():
            print(f"Processing area {area_id}: {value}")
            item = area_list.add()
            item.name = f'Area {area_id}'
            item.area_id = area_id
            
        print(f"Successfully loaded {len(area_list)} areas")
        
    except Exception as e:
        print(f"Error: {e}")
        import traceback
        traceback.print_exc()

if __name__ == '__main__':
    test_area_list_reload()