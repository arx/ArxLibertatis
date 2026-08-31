#!/usr/bin/env python3
"""
DLF File Comparison Tool

This script compares two DLF files to identify differences in their headers,
scene structures, and payload data. It's designed to help debug why an 
exported DLF might not work correctly in the game engine.
"""

import sys
import os
import struct
from ctypes import sizeof, create_string_buffer
import logging

# Add the current directory to path for imports
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

# Import data structures directly to avoid relative import issues
from dataCommon import (
    SavedVec3,
    SavedAnglef,
    PolyTypeFlag,
    SavedColor
)

# Re-implement the structures we need
from ctypes import (
    LittleEndianStructure,
    c_char,
    c_uint32,
    c_int16,
    c_int32,
    c_float
)

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

class DANAE_LS_INTER(LittleEndianStructure):
    _pack_ = 1
    _fields_ = [
        ("name",     c_char * 512),
        ("pos",      SavedVec3),
        ("angle",    SavedAnglef),
        ("ident",    c_int32),
        ("flags",    c_int32),
        ("pad",      c_int32 * 14),
        ("fpad",     c_float * 16),
    ]

class DANAE_LS_LIGHTINGHEADER(LittleEndianStructure):
    _pack_ = 1
    _fields_ = [
        ("nb_values", c_int32),
        ("ViewMode",  c_int32), # unused
        ("ModeLight", c_int32), # unused
        ("pad",       c_int32), # unused
    ]

# Import decompression utilities
from naivePkware import decompress_ftl
try:
    from lib import ArxIO
    ARXIO_AVAILABLE = True
except:
    ARXIO_AVAILABLE = False

def hex_dump(data, start_offset=0, max_bytes=256):
    """Create a hex dump of binary data"""
    result = []
    for i in range(0, min(len(data), max_bytes), 16):
        hex_bytes = ' '.join(f'{b:02x}' for b in data[i:i+16])
        ascii_chars = ''.join(chr(b) if 32 <= b <= 126 else '.' for b in data[i:i+16])
        result.append(f'{start_offset + i:08x}: {hex_bytes:<48} |{ascii_chars}|')
    if len(data) > max_bytes:
        result.append(f'... ({len(data) - max_bytes} more bytes)')
    return '\n'.join(result)

def compare_headers(header1, header2):
    """Compare two DANAE_LS_HEADER structures"""
    print("=== HEADER COMPARISON ===")
    
    fields = [
        'version', 'ident', 'lastuser', 'time', 'pos_edit', 'angle_edit',
        'nb_scn', 'nb_inter', 'nb_nodes', 'nb_nodeslinks', 'nb_zones',
        'lighting', 'nb_lights', 'nb_fogs', 'nb_bkgpolys', 'nb_ignoredpolys',
        'nb_childpolys', 'nb_paths', 'offset'
    ]
    
    differences = []
    for field in fields:
        val1 = getattr(header1, field)
        val2 = getattr(header2, field)
        
        if field == 'ident':
            val1_str = val1[:16].decode('iso-8859-1', errors='ignore').rstrip('\x00')
            val2_str = val2[:16].decode('iso-8859-1', errors='ignore').rstrip('\x00')
            if val1_str != val2_str:
                differences.append(f"  {field}: '{val1_str}' vs '{val2_str}'")
            else:
                print(f"  {field}: '{val1_str}' ✓")
        elif field == 'lastuser':
            val1_str = val1[:256].decode('iso-8859-1', errors='ignore').rstrip('\x00')
            val2_str = val2[:256].decode('iso-8859-1', errors='ignore').rstrip('\x00')
            if val1_str != val2_str:
                differences.append(f"  {field}: '{val1_str[:50]}...' vs '{val2_str[:50]}...'")
            else:
                print(f"  {field}: '{val1_str[:50]}...' ✓")
        elif hasattr(val1, 'x'):  # Vector3 types
            if (val1.x != val2.x) or (val1.y != val2.y) or (val1.z != val2.z):
                differences.append(f"  {field}: ({val1.x}, {val1.y}, {val1.z}) vs ({val2.x}, {val2.y}, {val2.z})")
            else:
                print(f"  {field}: ({val1.x}, {val1.y}, {val1.z}) ✓")
        elif hasattr(val1, 'a'):  # Angle types
            if (val1.a != val2.a) or (val1.b != val2.b) or (val1.g != val2.g):
                differences.append(f"  {field}: ({val1.a}, {val1.b}, {val1.g}) vs ({val2.a}, {val2.b}, {val2.g})")
            else:
                print(f"  {field}: ({val1.a}, {val1.b}, {val1.g}) ✓")
        else:
            if val1 != val2:
                differences.append(f"  {field}: {val1} vs {val2}")
            else:
                print(f"  {field}: {val1} ✓")
    
    if differences:
        print("DIFFERENCES FOUND:")
        for diff in differences:
            print(diff)
    else:
        print("Headers are identical!")
    
    return len(differences) == 0

def compare_scenes(scene1, scene2):
    """Compare two DANAE_LS_SCENE structures"""
    print("\n=== SCENE COMPARISON ===")
    
    name1 = scene1.name[:512].decode('iso-8859-1', errors='ignore').rstrip('\x00')
    name2 = scene2.name[:512].decode('iso-8859-1', errors='ignore').rstrip('\x00')
    
    print(f"Scene 1 name: '{name1}'")
    print(f"Scene 2 name: '{name2}'")
    
    if name1 != name2:
        print("❌ SCENE NAMES DIFFER!")
        print(f"Name 1 bytes: {scene1.name[:len(name1)+5]}")
        print(f"Name 2 bytes: {scene2.name[:len(name2)+5]}")
        
        # Show hex dump around the name field
        print("\nScene 1 name field hex dump:")
        print(hex_dump(bytes(scene1.name[:64])))
        print("\nScene 2 name field hex dump:")
        print(hex_dump(bytes(scene2.name[:64])))
        
        return False
    else:
        print("✓ Scene names match")
        return True

def compare_entities(entities1, entities2):
    """Compare entity lists"""
    print(f"\n=== ENTITIES COMPARISON ===")
    print(f"Entity count: {len(entities1)} vs {len(entities2)}")
    
    if len(entities1) != len(entities2):
        print("❌ Different number of entities!")
        return False
    
    differences = 0
    for i, (e1, e2) in enumerate(zip(entities1, entities2)):
        name1 = e1.name[:512].decode('iso-8859-1', errors='ignore').rstrip('\x00')
        name2 = e2.name[:512].decode('iso-8859-1', errors='ignore').rstrip('\x00')
        
        if name1 != name2:
            print(f"  Entity {i}: name differs - '{name1}' vs '{name2}'")
            differences += 1
        elif (e1.pos.x != e2.pos.x or e1.pos.y != e2.pos.y or e1.pos.z != e2.pos.z):
            print(f"  Entity {i} ({name1}): position differs")
            differences += 1
        elif (e1.angle.a != e2.angle.a or e1.angle.b != e2.angle.b or e1.angle.g != e2.angle.g):
            print(f"  Entity {i} ({name1}): angle differs")
            differences += 1
        elif e1.ident != e2.ident or e1.flags != e2.flags:
            print(f"  Entity {i} ({name1}): ident/flags differ")
            differences += 1
    
    if differences == 0:
        print("✓ All entities match")
        return True
    else:
        print(f"❌ {differences} entity differences found")
        return False

def compare_raw_data(data1, data2, name):
    """Compare raw binary data sections"""
    print(f"\n=== {name.upper()} COMPARISON ===")
    print(f"Size: {len(data1)} vs {len(data2)} bytes")
    
    if len(data1) != len(data2):
        print(f"❌ Different sizes!")
        return False
    
    if data1 == data2:
        print("✓ Data identical")
        return True
    else:
        print("❌ Data differs")
        # Find first difference
        for i, (b1, b2) in enumerate(zip(data1, data2)):
            if b1 != b2:
                print(f"First difference at offset {i:08x}: {b1:02x} vs {b2:02x}")
                # Show context around the difference
                start = max(0, i - 16)
                end = min(len(data1), i + 17)
                print("Data 1 context:")
                print(hex_dump(data1[start:end], start))
                print("Data 2 context:")
                print(hex_dump(data2[start:end], start))
                break
        return False

def parse_uncompressed_dlf(data, header):
    """Parse the uncompressed DLF payload"""
    pos = 0
    
    # Parse scene
    scene = DANAE_LS_SCENE.from_buffer_copy(data, pos)
    pos += sizeof(DANAE_LS_SCENE)
    
    # Parse entities
    entities = []
    EntitiesType = DANAE_LS_INTER * header.nb_inter
    if header.nb_inter > 0:
        entities_data = EntitiesType.from_buffer_copy(data, pos)
        entities = list(entities_data)
        pos += sizeof(EntitiesType)
    
    # Handle lighting data
    lighting_data = b""
    if header.lighting != 0:
        lighting_header = DANAE_LS_LIGHTINGHEADER.from_buffer_copy(data, pos)
        lighting_header_size = sizeof(DANAE_LS_LIGHTINGHEADER)
        lighting_values_size = lighting_header.nb_values * 4
        lighting_data = data[pos:pos + lighting_header_size + lighting_values_size]
        pos += lighting_header_size + lighting_values_size
    
    # Skip lights, fogs, nodes data for now - we just want scene and entity data
    # which are the most critical for FTS file loading
    
    return {
        'scene': scene,
        'entities': entities,
        'lighting_data': lighting_data,
        'nodes_data': b""  # Not parsing for this comparison
    }

def analyze_dlf_file(filepath):
    """Analyze a single DLF file and return its components"""
    print(f"\nAnalyzing: {filepath}")
    
    if not os.path.exists(filepath):
        raise FileNotFoundError(f"File not found: {filepath}")
    
    with open(filepath, 'rb') as f:
        raw_data = f.read()
    
    print(f"File size: {len(raw_data)} bytes")
    
    # Parse header
    header = DANAE_LS_HEADER.from_buffer_copy(raw_data, 0)
    header_size = sizeof(DANAE_LS_HEADER)
    
    print(f"Header size: {header_size} bytes")
    print(f"Version: {header.version}")
    print(f"Ident: {header.ident[:16].decode('iso-8859-1', errors='ignore').rstrip()}")
    
    # Compressed payload starts after header
    compressed_payload = raw_data[header_size:]
    print(f"Compressed payload size: {len(compressed_payload)} bytes")
    
    # Try decompression with multiple methods
    uncompressed = None
    try:
        # First try the ArxIO library if available
        if ARXIO_AVAILABLE:
            arxio = ArxIO()
            uncompressed = arxio.unpack(compressed_payload)
            print(f"✓ Decompressed with ArxIO: {len(uncompressed)} bytes")
        else:
            raise Exception("ArxIO not available")
    except Exception as e:
        print(f"ArxIO decompression failed: {e}")
        try:
            # Fallback to naive PKWare
            uncompressed = decompress_ftl(compressed_payload)
            print(f"✓ Decompressed with naive PKWare: {len(uncompressed)} bytes")
        except Exception as e2:
            print(f"❌ Both decompression methods failed!")
            print(f"ArxIO error: {e}")
            print(f"PKWare error: {e2}")
            raise
    
    # Parse the uncompressed payload
    dlf_data = parse_uncompressed_dlf(uncompressed, header)
    
    return {
        'filepath': filepath,
        'raw_data': raw_data,
        'header': header,
        'compressed_payload': compressed_payload,
        'uncompressed_payload': uncompressed,
        'dlf_data': dlf_data
    }

def main():
    if len(sys.argv) != 3:
        print("Usage: python dlf_comparison.py <original_dlf> <exported_dlf>")
        print("Example: python dlf_comparison.py level1_original.dlf level1_exported.dlf")
        sys.exit(1)
    
    original_path = sys.argv[1]
    exported_path = sys.argv[2]
    
    print("DLF File Comparison Tool")
    print("=" * 50)
    
    try:
        # Analyze both files
        original = analyze_dlf_file(original_path)
        exported = analyze_dlf_file(exported_path)
        
        print("\n" + "=" * 50)
        print("COMPARISON RESULTS")
        print("=" * 50)
        
        # Compare headers
        headers_match = compare_headers(original['header'], exported['header'])
        
        # Compare scenes (the critical part for FTS loading!)
        scenes_match = compare_scenes(original['dlf_data']['scene'], exported['dlf_data']['scene'])
        
        # Compare entities
        entities_match = compare_entities(original['dlf_data']['entities'], exported['dlf_data']['entities'])
        
        # Compare other data sections
        lighting_match = compare_raw_data(
            original['dlf_data']['lighting_data'], 
            exported['dlf_data']['lighting_data'], 
            "lighting data"
        )
        
        nodes_match = compare_raw_data(
            original['dlf_data']['nodes_data'],
            exported['dlf_data']['nodes_data'],
            "nodes data"
        )
        
        # Summary
        print(f"\n{'='*50}")
        print("SUMMARY")
        print(f"{'='*50}")
        print(f"Headers match: {'✓' if headers_match else '❌'}")
        print(f"Scenes match: {'✓' if scenes_match else '❌'}")
        print(f"Entities match: {'✓' if entities_match else '❌'}")
        print(f"Lighting data match: {'✓' if lighting_match else '❌'}")
        print(f"Nodes data match: {'✓' if nodes_match else '❌'}")
        
        if not scenes_match:
            print("\n⚠️  CRITICAL: Scene name mismatch detected!")
            print("This is likely why the engine cannot find the FTS file.")
            print("The engine uses the scene name/path to locate the corresponding FTS file.")
        
        if all([headers_match, scenes_match, entities_match, lighting_match, nodes_match]):
            print("\n🎉 Files are functionally identical!")
        else:
            print(f"\n❌ Files differ in {sum([not headers_match, not scenes_match, not entities_match, not lighting_match, not nodes_match])} areas")
        
    except Exception as e:
        print(f"\n❌ Error during comparison: {e}")
        import traceback
        traceback.print_exc()
        sys.exit(1)

if __name__ == "__main__":
    main()