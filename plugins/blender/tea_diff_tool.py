#!/usr/bin/env python3

import sys
import os
import argparse
from pathlib import Path

# Add the plugin directory to the path
sys.path.append('/home/burner/Desktop/ArxLibertatis/plugins/blender')

from arx_addon.dataTea import TeaSerializer

def analyze_tea_file(file_path):
    """Analyze a TEA file and return detailed information."""
    try:
        serializer = TeaSerializer()
        frames = serializer.read(file_path)
        
        if not frames:
            return None
        
        analysis = {
            'path': file_path,
            'frame_count': len(frames),
            'total_duration': sum(frame.duration for frame in frames),
            'has_translation': any(frame.translation is not None for frame in frames),
            'has_rotation': any(frame.rotation is not None for frame in frames),
            'group_count': len(frames[0].groups) if frames else 0,
            'active_groups': set(),
            'flags': set(),
            'sample_names': set()
        }
        
        # Analyze frames
        for frame in frames:
            analysis['flags'].add(frame.flags)
            if frame.sampleName:
                analysis['sample_names'].add(frame.sampleName)
            
            # Check active groups
            for i, group in enumerate(frame.groups):
                if group.key_group != -1:
                    analysis['active_groups'].add(i)
        
        return analysis
        
    except Exception as e:
        print(f"Error analyzing {file_path}: {e}")
        return None

def compare_tea_files_detailed(file1_path, file2_path):
    """Compare two TEA files in detail."""
    print(f"Comparing TEA files:")
    print(f"  File 1: {file1_path}")
    print(f"  File 2: {file2_path}")
    print("")
    
    # Analyze both files
    analysis1 = analyze_tea_file(file1_path)
    analysis2 = analyze_tea_file(file2_path)
    
    if not analysis1 or not analysis2:
        print("Failed to analyze one or both files")
        return False
    
    # Compare basic properties
    print("=== BASIC COMPARISON ===")
    print(f"Frame count:     {analysis1['frame_count']:4d} vs {analysis2['frame_count']:4d}")
    print(f"Total duration:  {analysis1['total_duration']:7.3f}s vs {analysis2['total_duration']:7.3f}s")
    print(f"Group count:     {analysis1['group_count']:4d} vs {analysis2['group_count']:4d}")
    print(f"Has translation: {analysis1['has_translation']} vs {analysis2['has_translation']}")
    print(f"Has rotation:    {analysis1['has_rotation']} vs {analysis2['has_rotation']}")
    print(f"Active groups:   {len(analysis1['active_groups']):4d} vs {len(analysis2['active_groups']):4d}")
    print(f"Flags:           {analysis1['flags']} vs {analysis2['flags']}")
    print(f"Sample names:    {analysis1['sample_names']} vs {analysis2['sample_names']}")
    print("")
    
    # Detailed frame-by-frame comparison
    try:
        serializer = TeaSerializer()
        frames1 = serializer.read(file1_path)
        frames2 = serializer.read(file2_path)
        
        if len(frames1) != len(frames2):
            print(f"WARNING: Frame count mismatch ({len(frames1)} vs {len(frames2)})")
            return False
        
        print("=== DETAILED FRAME COMPARISON ===")
        max_diffs_to_show = 10
        diff_count = 0
        
        for i, (frame1, frame2) in enumerate(zip(frames1, frames2)):
            frame_diffs = []
            
            # Check duration
            if abs(frame1.duration - frame2.duration) > 0.001:
                frame_diffs.append(f"duration: {frame1.duration:.3f} vs {frame2.duration:.3f}")
            
            # Check flags
            if frame1.flags != frame2.flags:
                frame_diffs.append(f"flags: {frame1.flags} vs {frame2.flags}")
            
            # Check translation
            if frame1.translation and frame2.translation:
                if (abs(frame1.translation.x - frame2.translation.x) > 0.001 or
                    abs(frame1.translation.y - frame2.translation.y) > 0.001 or
                    abs(frame1.translation.z - frame2.translation.z) > 0.001):
                    frame_diffs.append(f"translation: ({frame1.translation.x:.3f}, {frame1.translation.y:.3f}, {frame1.translation.z:.3f}) vs ({frame2.translation.x:.3f}, {frame2.translation.y:.3f}, {frame2.translation.z:.3f})")
            elif frame1.translation != frame2.translation:
                frame_diffs.append(f"translation: {frame1.translation} vs {frame2.translation}")
            
            # Check rotation
            if frame1.rotation and frame2.rotation:
                if (abs(frame1.rotation.w - frame2.rotation.w) > 0.001 or
                    abs(frame1.rotation.x - frame2.rotation.x) > 0.001 or
                    abs(frame1.rotation.y - frame2.rotation.y) > 0.001 or
                    abs(frame1.rotation.z - frame2.rotation.z) > 0.001):
                    frame_diffs.append(f"rotation: ({frame1.rotation.w:.3f}, {frame1.rotation.x:.3f}, {frame1.rotation.y:.3f}, {frame1.rotation.z:.3f}) vs ({frame2.rotation.w:.3f}, {frame2.rotation.x:.3f}, {frame2.rotation.y:.3f}, {frame2.rotation.z:.3f})")
            elif frame1.rotation != frame2.rotation:
                frame_diffs.append(f"rotation: {frame1.rotation} vs {frame2.rotation}")
            
            # Check groups
            if len(frame1.groups) != len(frame2.groups):
                frame_diffs.append(f"group count: {len(frame1.groups)} vs {len(frame2.groups)}")
            else:
                for j, (group1, group2) in enumerate(zip(frame1.groups, frame2.groups)):
                    group_diffs = []
                    
                    if group1.key_group != group2.key_group:
                        group_diffs.append(f"key_group: {group1.key_group} vs {group2.key_group}")
                    
                    if (abs(group1.translate.x - group2.translate.x) > 0.001 or
                        abs(group1.translate.y - group2.translate.y) > 0.001 or
                        abs(group1.translate.z - group2.translate.z) > 0.001):
                        group_diffs.append(f"translate: ({group1.translate.x:.3f}, {group1.translate.y:.3f}, {group1.translate.z:.3f}) vs ({group2.translate.x:.3f}, {group2.translate.y:.3f}, {group2.translate.z:.3f})")
                    
                    if (abs(group1.Quaternion.w - group2.Quaternion.w) > 0.001 or
                        abs(group1.Quaternion.x - group2.Quaternion.x) > 0.001 or
                        abs(group1.Quaternion.y - group2.Quaternion.y) > 0.001 or
                        abs(group1.Quaternion.z - group2.Quaternion.z) > 0.001):
                        group_diffs.append(f"quat: ({group1.Quaternion.w:.3f}, {group1.Quaternion.x:.3f}, {group1.Quaternion.y:.3f}, {group1.Quaternion.z:.3f}) vs ({group2.Quaternion.w:.3f}, {group2.Quaternion.x:.3f}, {group2.Quaternion.y:.3f}, {group2.Quaternion.z:.3f})")
                    
                    if (abs(group1.zoom.x - group2.zoom.x) > 0.001 or
                        abs(group1.zoom.y - group2.zoom.y) > 0.001 or
                        abs(group1.zoom.z - group2.zoom.z) > 0.001):
                        group_diffs.append(f"scale: ({group1.zoom.x:.3f}, {group1.zoom.y:.3f}, {group1.zoom.z:.3f}) vs ({group2.zoom.x:.3f}, {group2.zoom.y:.3f}, {group2.zoom.z:.3f})")
                    
                    if group_diffs:
                        frame_diffs.append(f"grp{j}: {'; '.join(group_diffs)}")
            
            if frame_diffs:
                diff_count += 1
                if diff_count <= max_diffs_to_show:
                    print(f"Frame {i:3d}: {'; '.join(frame_diffs)}")
        
        if diff_count > max_diffs_to_show:
            print(f"... and {diff_count - max_diffs_to_show} more frames with differences")
        
        if diff_count == 0:
            print("No significant differences found!")
        else:
            print(f"Total frames with differences: {diff_count}/{len(frames1)}")
        
        return diff_count == 0
        
    except Exception as e:
        print(f"Error during detailed comparison: {e}")
        return False

def main():
    parser = argparse.ArgumentParser(description='TEA Animation File Diff Tool')
    parser.add_argument('file1', help='First TEA file')
    parser.add_argument('file2', help='Second TEA file')
    parser.add_argument('--verbose', '-v', action='store_true', help='Verbose output')
    
    args = parser.parse_args()
    
    if not os.path.exists(args.file1):
        print(f"Error: File '{args.file1}' not found")
        sys.exit(1)
    
    if not os.path.exists(args.file2):
        print(f"Error: File '{args.file2}' not found")
        sys.exit(1)
    
    # Compare the files
    success = compare_tea_files_detailed(args.file1, args.file2)
    
    if success:
        print("\nFiles are identical (within tolerance)")
        sys.exit(0)
    else:
        print("\nFiles have differences")
        sys.exit(1)

if __name__ == '__main__':
    main()