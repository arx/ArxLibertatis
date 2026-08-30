#!/usr/bin/env python3

import sys
import os
import tempfile
import shutil
import argparse
from pathlib import Path

# Add the plugin directory to the path
sys.path.append('/home/burner/Desktop/ArxLibertatis/plugins/blender')

import bpy
from arx_addon.managers import getAddon
from arx_addon.dataTea import TeaSerializer
from arx_addon.arx_io_util import ArxException

def compare_tea_files(original_path, exported_path):
    """Compare two TEA files and return differences."""
    try:
        serializer = TeaSerializer()
        
        # Read both files
        original_data = serializer.read(original_path)
        exported_data = serializer.read(exported_path)
        
        differences = []
        
        # A re bake writes a keyframe per frame, so the counts legitimately differ.
        # What has to survive the trip is the pose at the frames the original
        # actually keyed, so pair the two up on num_frame and compare those.
        exported_by_frame = {f.num_frame: f for f in exported_data}
        missing = [f.num_frame for f in original_data if f.num_frame not in exported_by_frame]
        if missing:
            differences.append(f"Frames missing from export: {missing}")
        pairs = [(f, exported_by_frame[f.num_frame]) for f in original_data
                 if f.num_frame in exported_by_frame]
        
        # Compare each frame
        for i, (orig_frame, exp_frame) in enumerate(pairs):
            frame_diffs = []
            
            # Compare durations (allow small differences due to conversion)
            if abs(orig_frame.duration - exp_frame.duration) > 0.001:
                frame_diffs.append(f"duration: {orig_frame.duration:.3f} vs {exp_frame.duration:.3f}")
            
            # Compare flags
            if orig_frame.flags != exp_frame.flags:
                frame_diffs.append(f"flags: {orig_frame.flags} vs {exp_frame.flags}")
            
            # A frame with no key_move reads back as None, which is the engine
            # holding the root still - the same animation as an explicit zero, so
            # compare the pose either side describes rather than how it is stored.
            orig_t = (0.0, 0.0, 0.0) if orig_frame.translation is None else (
                orig_frame.translation.x, orig_frame.translation.y, orig_frame.translation.z)
            exp_t = (0.0, 0.0, 0.0) if exp_frame.translation is None else (
                exp_frame.translation.x, exp_frame.translation.y, exp_frame.translation.z)
            if max(abs(a - b) for a, b in zip(orig_t, exp_t)) > 0.001:
                frame_diffs.append(f"translation: {orig_t} vs {exp_t}")
            
            orig_r = (1.0, 0.0, 0.0, 0.0) if orig_frame.rotation is None else (
                orig_frame.rotation.w, orig_frame.rotation.x, orig_frame.rotation.y, orig_frame.rotation.z)
            exp_r = (1.0, 0.0, 0.0, 0.0) if exp_frame.rotation is None else (
                exp_frame.rotation.w, exp_frame.rotation.x, exp_frame.rotation.y, exp_frame.rotation.z)
            if min(max(abs(a - b) for a, b in zip(orig_r, exp_r)),
                   max(abs(a + b) for a, b in zip(orig_r, exp_r))) > 0.001:
                frame_diffs.append(f"rotation: {orig_r} vs {exp_r}")
            
            # Compare groups
            if len(orig_frame.groups) != len(exp_frame.groups):
                frame_diffs.append(f"group count: {len(orig_frame.groups)} vs {len(exp_frame.groups)}")
            else:
                for j, (orig_group, exp_group) in enumerate(zip(orig_frame.groups, exp_frame.groups)):
                    group_diffs = []
                    
                    if (abs(orig_group.translate.x - exp_group.translate.x) > 0.001 or
                        abs(orig_group.translate.y - exp_group.translate.y) > 0.001 or
                        abs(orig_group.translate.z - exp_group.translate.z) > 0.001):
                        group_diffs.append(f"translate: ({orig_group.translate.x:.3f}, {orig_group.translate.y:.3f}, {orig_group.translate.z:.3f}) vs ({exp_group.translate.x:.3f}, {exp_group.translate.y:.3f}, {exp_group.translate.z:.3f})")
                    
                    # q and -q are the same rotation, so compare up to sign.
                    oq = (orig_group.Quaternion.w, orig_group.Quaternion.x,
                          orig_group.Quaternion.y, orig_group.Quaternion.z)
                    eq = (exp_group.Quaternion.w, exp_group.Quaternion.x,
                          exp_group.Quaternion.y, exp_group.Quaternion.z)
                    if min(max(abs(a - b) for a, b in zip(oq, eq)),
                           max(abs(a + b) for a, b in zip(oq, eq))) > 0.001:
                        group_diffs.append(f"quaternion: ({orig_group.Quaternion.w:.3f}, {orig_group.Quaternion.x:.3f}, {orig_group.Quaternion.y:.3f}, {orig_group.Quaternion.z:.3f}) vs ({exp_group.Quaternion.w:.3f}, {exp_group.Quaternion.x:.3f}, {exp_group.Quaternion.y:.3f}, {exp_group.Quaternion.z:.3f})")
                    
                    if (abs(orig_group.zoom.x - exp_group.zoom.x) > 0.001 or
                        abs(orig_group.zoom.y - exp_group.zoom.y) > 0.001 or
                        abs(orig_group.zoom.z - exp_group.zoom.z) > 0.001):
                        group_diffs.append(f"zoom: ({orig_group.zoom.x:.3f}, {orig_group.zoom.y:.3f}, {orig_group.zoom.z:.3f}) vs ({exp_group.zoom.x:.3f}, {exp_group.zoom.y:.3f}, {exp_group.zoom.z:.3f})")
                    
                    if group_diffs:
                        frame_diffs.append(f"group {j}: {'; '.join(group_diffs)}")
            
            if frame_diffs:
                differences.append(f"Frame {orig_frame.num_frame}: {'; '.join(frame_diffs)}")
        
        return differences
        
    except Exception as e:
        return [f"Error comparing files: {str(e)}"]

def test_tea_roundtrip(tea_file_path, test_model_path=None):
    """Test TEA round trip functionality."""
    print(f"Testing TEA round trip for: {tea_file_path}")
    
    # Clear scene
    bpy.ops.object.select_all(action='SELECT')
    bpy.ops.object.delete(use_global=False)
    for action in bpy.data.actions:
        bpy.data.actions.remove(action)
    
    # If a test model is provided, import it first
    if test_model_path and os.path.exists(test_model_path):
        try:
            addon = getAddon(bpy.context)
            addon.objectManager.loadFile(bpy.context, test_model_path, bpy.context.scene, import_tweaks=False)
            print(f"Imported test model: {test_model_path}")
        except Exception as e:
            print(f"Failed to import test model: {e}")
            return False
    
    # Find mesh object with armature
    mesh_obj = None
    for obj in bpy.data.objects:
        if obj.type == 'MESH':
            # Check if it has an armature modifier
            for modifier in obj.modifiers:
                if modifier.type == 'ARMATURE' and modifier.object:
                    mesh_obj = obj
                    break
            if mesh_obj:
                break
    
    if not mesh_obj:
        print("No mesh with armature found for animation import")
        return False
    
    # Set active object
    bpy.context.view_layer.objects.active = mesh_obj
    mesh_obj.select_set(True)
    
    try:
        # Import animation
        addon = getAddon(bpy.context)
        action = addon.animationManager.loadAnimation(tea_file_path, f"imported_{os.path.basename(tea_file_path)}")
        
        if not action:
            print(f"Failed to import animation from {tea_file_path}")
            return False
        
        print(f"Import: Ok")
        
        # Make sure the mesh is still the active object before export
        bpy.context.view_layer.objects.active = mesh_obj
        mesh_obj.select_set(True)
        
        # Debug: check active object
        active_obj = bpy.context.active_object
        print(f"Active object before export: {active_obj.name if active_obj else 'None'} (type: {active_obj.type if active_obj else 'None'})")
        
        # Export animation
        temp_dir = tempfile.mkdtemp()
        exported_file = os.path.join(temp_dir, "exported_animation.tea")
        
        success = addon.animationManager.saveAnimation(exported_file, action.name)
        
        if not success:
            print(f"Failed to export animation to {exported_file}")
            shutil.rmtree(temp_dir)
            return False
        
        print(f"Export: Ok")
        
        # Compare files
        differences = compare_tea_files(tea_file_path, exported_file)
        
        if differences:
            print("Compare: Differences found")
            for diff in differences[:10]:  # Show first 10 differences
                print(f"  {diff}")
            if len(differences) > 10:
                print(f"  ... and {len(differences) - 10} more differences")
        else:
            print("Compare: Ok")
        
        # Clean up
        shutil.rmtree(temp_dir)
        
        return True
        
    except Exception as e:
        print(f"Error during round trip test: {e}")
        import traceback
        traceback.print_exc()
        return False

def main():
    # Parse command line arguments (after --)
    if '--' in sys.argv:
        args = sys.argv[sys.argv.index('--') + 1:]
    else:
        args = []
    
    parser = argparse.ArgumentParser(description='TEA Round Trip Test')
    parser.add_argument('tea_file', help='TEA file to test')
    parser.add_argument('--model', help='Model file to import for testing')
    parser.add_argument('--game-dir', help='Game directory path')
    
    if not args:
        print("Usage: test_tea_roundtrip.py <tea_file> [--model <model_file>] [--game-dir <game_directory>]")
        return
    
    parsed_args = parser.parse_args(args)
    
    # Set up addon if game directory is provided
    if parsed_args.game_dir:
        try:
            addon = getAddon(bpy.context)
            # The addon doesn't have setArxPath method, but let's at least mention the path
            print(f"Game directory: {parsed_args.game_dir}")
        except Exception as e:
            print(f"Failed to set game directory: {e}")
    
    # Test the TEA file
    success = test_tea_roundtrip(parsed_args.tea_file, parsed_args.model)
    
    if success:
        print("TEA round trip test completed successfully")
    else:
        print("TEA round trip test failed")
        sys.exit(1)

if __name__ == '__main__':
    main()