#!/usr/bin/env python3

import sys
import os
sys.path.append('/home/burner/Desktop/ArxLibertatis/plugins/blender')

import bpy
from arx_addon.managers import getAddon
from arx_addon.arx_ui_area import importArea

# Test if area import works
def test_area_import():
    try:
        # Test importing area 0
        area_id = 0
        print(f"Testing import of area {area_id}")
        
        def mock_report(level, message):
            print(f"REPORT {level}: {message}")
        
        importArea(bpy.context, mock_report, area_id)
        print(f"Successfully imported area {area_id}")
        
        # Check if scene was created
        scene_name = f"Area_{area_id:02d}"
        scene = bpy.data.scenes.get(scene_name)
        if scene:
            print(f"Scene '{scene_name}' created successfully")
            print(f"Scene objects: {len(scene.objects)}")
        else:
            print(f"ERROR: Scene '{scene_name}' not found")
            
    except Exception as e:
        print(f"Error: {e}")
        import traceback
        traceback.print_exc()

if __name__ == '__main__':
    test_area_import()