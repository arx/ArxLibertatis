#!/usr/bin/env python3
"""
TEA file debugging utility.
Helps diagnose corrupted or problematic TEA animation files.
"""

import sys
import os
from struct import unpack

def analyze_tea_file(filepath):
    """Analyze a TEA file and show its structure."""
    if not os.path.exists(filepath):
        print(f"Error: File {filepath} not found")
        return
    
    with open(filepath, 'rb') as f:
        data = f.read()
    
    print(f"File: {filepath}")
    print(f"Size: {len(data)} bytes")
    
    if len(data) < 292:  # Minimum size for THEA_HEADER
        print("ERROR: File too small for valid TEA header")
        return
    
    # Read header
    identity = data[0:20]
    version, = unpack('<L', data[20:24])
    anim_name = data[24:280]
    nb_frames, nb_groups, nb_key_frames = unpack('<lll', data[280:292])
    
    print(f"\nHeader:")
    print(f"  Identity: {identity}")
    print(f"  Version: {version}")
    print(f"  Animation Name: {anim_name.split(b'\\x00')[0].decode('iso-8859-1', errors='replace')}")
    print(f"  Frames: {nb_frames}")
    print(f"  Groups: {nb_groups}")
    print(f"  Key Frames: {nb_key_frames}")
    
    # Validate header
    if version not in [2014, 2015]:
        print(f"WARNING: Unusual version {version}")
    if nb_frames < 0 or nb_frames > 100000:
        print(f"WARNING: Suspicious nb_frames {nb_frames}")
    if nb_groups < 0 or nb_groups > 1000:
        print(f"WARNING: Suspicious nb_groups {nb_groups}")
    if nb_key_frames < 0 or nb_key_frames > 10000:
        print(f"WARNING: Suspicious nb_key_frames {nb_key_frames}")
    
    # Analyze keyframes
    pos = 292
    keyframe_size = 264 if version == 2015 else 32  # THEA_KEYFRAME_2015 vs 2014
    
    print(f"\nKeyframes:")
    for i in range(min(nb_key_frames, 10)):  # Show first 10 keyframes
        if pos + keyframe_size > len(data):
            print(f"  Frame {i}: TRUNCATED - need {keyframe_size} bytes, only {len(data) - pos} available")
            break
            
        if version == 2015:
            num_frame, flag_frame = unpack('<ll', data[pos:pos+8])
            info_frame = data[pos+8:pos+264]
            master_key_frame, key_frame, key_move, key_orient, key_morph, time_frame = unpack('<llllll', data[pos+264:pos+288])
        else:
            num_frame, flag_frame, master_key_frame, key_frame, key_move, key_orient, key_morph, time_frame = unpack('<llllllll', data[pos:pos+32])
            info_frame = b""
        
        print(f"  Frame {i}:")
        print(f"    num_frame: {num_frame}")
        print(f"    flag_frame: {flag_frame}")
        if flag_frame not in [-1, 9] and abs(flag_frame) > 1000:
            print(f"    *** WARNING: Invalid flag_frame {flag_frame} ***")
        print(f"    time_frame: {time_frame} ({'%.6f' % (time_frame/1000000.0)}s)")
        print(f"    key_move: {key_move}, key_orient: {key_orient}, key_morph: {key_morph}")
        if version == 2015 and info_frame:
            info_str = info_frame.split(b'\\x00')[0].decode('iso-8859-1', errors='replace')
            if info_str:
                print(f"    info_frame: '{info_str}'")
        
        pos += keyframe_size
        
        # Skip variable-length data for this frame
        # This is a simplified analysis - real parsing would be more complex
        
    if nb_key_frames > 10:
        print(f"  ... and {nb_key_frames - 10} more keyframes")

if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("Usage: python3 tea_debug.py <tea_file>")
        sys.exit(1)
    
    analyze_tea_file(sys.argv[1])