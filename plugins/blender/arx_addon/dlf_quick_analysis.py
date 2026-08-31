#!/usr/bin/env python3
"""
Quick DLF Analysis Tool - Focus on critical differences
"""

import sys
import os
from ctypes import sizeof
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

from dataCommon import SavedVec3, SavedAnglef
from ctypes import LittleEndianStructure, c_char, c_int32, c_float

class DANAE_LS_HEADER(LittleEndianStructure):
    _pack_ = 1
    _fields_ = [
        ("version",         c_float),
        ("ident",           c_char * 16),
        ("lastuser",        c_char * 256),
        ("time",            c_int32),
        ("pos_edit",        SavedVec3),
        ("angle_edit",      SavedAnglef),
        ("nb_scn",          c_int32),
        ("nb_inter",        c_int32),
        ("nb_nodes",        c_int32),
        ("nb_nodeslinks",   c_int32),
        ("nb_zones",        c_int32),
        ("lighting",        c_int32),
        ("Bpad",            c_int32 * 256),
        ("nb_lights",       c_int32),
        ("nb_fogs",         c_int32),
        ("nb_bkgpolys",     c_int32),
        ("nb_ignoredpolys", c_int32),
        ("nb_childpolys",   c_int32),
        ("nb_paths",        c_int32),
        ("pad",             c_int32 * 250),
        ("offset",          SavedVec3),
        ("fpad",            c_float * 253),
        ("cpad",            c_char * 4096),
        ("bpad",            c_int32 * 256),
    ]

class DANAE_LS_SCENE(LittleEndianStructure):
    _pack_ = 1
    _fields_ = [
        ("name", c_char * 512),
        ("pad",  c_int32 * 16),
        ("fpad", c_float * 16)
    ]

from naivePkware import decompress_ftl
try:
    from lib import ArxIO
    ARXIO_AVAILABLE = True
except:
    ARXIO_AVAILABLE = False

def analyze_file(filepath):
    """Quick analysis of a DLF file"""
    with open(filepath, 'rb') as f:
        raw_data = f.read()
    
    # Parse header
    header = DANAE_LS_HEADER.from_buffer_copy(raw_data, 0)
    header_size = sizeof(DANAE_LS_HEADER)
    
    # Get compressed payload
    compressed_payload = raw_data[header_size:]
    
    # Decompress
    try:
        if ARXIO_AVAILABLE:
            arxio = ArxIO()
            uncompressed = arxio.unpack(compressed_payload)
        else:
            uncompressed = decompress_ftl(compressed_payload)
    except Exception as e:
        return None, None, str(e)
    
    # Parse scene
    scene = DANAE_LS_SCENE.from_buffer_copy(uncompressed, 0)
    
    return header, scene, None

def main():
    if len(sys.argv) != 3:
        print("Usage: python dlf_quick_analysis.py <original_dlf> <exported_dlf>")
        sys.exit(1)
    
    original_path = sys.argv[1]
    exported_path = sys.argv[2]
    
    print("Quick DLF Analysis")
    print("=" * 40)
    
    # Analyze original
    print(f"Original: {os.path.basename(original_path)}")
    orig_header, orig_scene, orig_error = analyze_file(original_path)
    if orig_error:
        print(f"❌ Error: {orig_error}")
        return
    
    orig_scene_name = orig_scene.name[:512].decode('iso-8859-1', errors='ignore').rstrip('\x00')
    orig_lastuser = orig_header.lastuser[:256].decode('iso-8859-1', errors='ignore').rstrip('\x00')
    print(f"  Scene name: '{orig_scene_name}'")
    print(f"  Last user: '{orig_lastuser}'")
    print(f"  Timestamp: {orig_header.time}")
    print(f"  Entities: {orig_header.nb_inter}")
    
    # Analyze exported
    print(f"\nExported: {os.path.basename(exported_path)}")
    exp_header, exp_scene, exp_error = analyze_file(exported_path)
    if exp_error:
        print(f"❌ Error: {exp_error}")
        return
    
    exp_scene_name = exp_scene.name[:512].decode('iso-8859-1', errors='ignore').rstrip('\x00')
    exp_lastuser = exp_header.lastuser[:256].decode('iso-8859-1', errors='ignore').rstrip('\x00')
    print(f"  Scene name: '{exp_scene_name}'")
    print(f"  Last user: '{exp_lastuser}'")
    print(f"  Timestamp: {exp_header.time}")
    print(f"  Entities: {exp_header.nb_inter}")
    
    # Compare critical fields
    print(f"\n{'='*40}")
    print("CRITICAL COMPARISON")
    print(f"{'='*40}")
    
    if orig_scene_name == exp_scene_name:
        print("✅ Scene names MATCH")
        print("   This is GOOD - engine should find the FTS file")
    else:
        print("❌ Scene names DIFFER!")
        print("   This could prevent FTS file loading!")
        print(f"   Original: '{orig_scene_name}'")
        print(f"   Exported: '{exp_scene_name}'")
    
    if orig_header.nb_inter == exp_header.nb_inter:
        print("✅ Entity counts MATCH")
    else:
        print(f"❌ Entity counts DIFFER: {orig_header.nb_inter} vs {exp_header.nb_inter}")
    
    # Show hex dumps of scene name field if they differ
    if orig_scene_name != exp_scene_name:
        print("\nScene name field hex dumps:")
        print("Original:")
        orig_bytes = bytes(orig_scene.name[:64])
        for i in range(0, len(orig_bytes), 16):
            hex_str = ' '.join(f'{b:02x}' for b in orig_bytes[i:i+16])
            ascii_str = ''.join(chr(b) if 32 <= b <= 126 else '.' for b in orig_bytes[i:i+16])
            print(f"  {i:04x}: {hex_str:<48} |{ascii_str}|")
        
        print("Exported:")
        exp_bytes = bytes(exp_scene.name[:64])
        for i in range(0, len(exp_bytes), 16):
            hex_str = ' '.join(f'{b:02x}' for b in exp_bytes[i:i+16])
            ascii_str = ''.join(chr(b) if 32 <= b <= 126 else '.' for b in exp_bytes[i:i+16])
            print(f"  {i:04x}: {hex_str:<48} |{ascii_str}|")

if __name__ == "__main__":
    main()