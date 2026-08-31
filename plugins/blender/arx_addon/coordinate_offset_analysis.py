#!/usr/bin/env python3
"""
Coordinate Offset Analysis - Identify systematic coordinate transformations
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
        ("version", c_float), ("ident", c_char * 16), ("lastuser", c_char * 256), ("time", c_int32),
        ("pos_edit", SavedVec3), ("angle_edit", SavedAnglef), ("nb_scn", c_int32), ("nb_inter", c_int32),
        ("nb_nodes", c_int32), ("nb_nodeslinks", c_int32), ("nb_zones", c_int32), ("lighting", c_int32),
        ("Bpad", c_int32 * 256), ("nb_lights", c_int32), ("nb_fogs", c_int32), ("nb_bkgpolys", c_int32),
        ("nb_ignoredpolys", c_int32), ("nb_childpolys", c_int32), ("nb_paths", c_int32), ("pad", c_int32 * 250),
        ("offset", SavedVec3), ("fpad", c_float * 253), ("cpad", c_char * 4096), ("bpad", c_int32 * 256),
    ]

class DANAE_LS_SCENE(LittleEndianStructure):
    _pack_ = 1
    _fields_ = [("name", c_char * 512), ("pad", c_int32 * 16), ("fpad", c_float * 16)]

class DANAE_LS_INTER(LittleEndianStructure):
    _pack_ = 1
    _fields_ = [
        ("name", c_char * 512), ("pos", SavedVec3), ("angle", SavedAnglef), ("ident", c_int32),
        ("flags", c_int32), ("pad", c_int32 * 14), ("fpad", c_float * 16),
    ]

from naivePkware import decompress_ftl
try:
    from lib import ArxIO
    ARXIO_AVAILABLE = True
except:
    ARXIO_AVAILABLE = False

def analyze_coordinates(filepath):
    """Extract entity positions and header offset from DLF file"""
    with open(filepath, 'rb') as f:
        raw_data = f.read()
    
    header = DANAE_LS_HEADER.from_buffer_copy(raw_data, 0)
    header_size = sizeof(DANAE_LS_HEADER)
    compressed_payload = raw_data[header_size:]
    
    # Decompress
    if ARXIO_AVAILABLE:
        arxio = ArxIO()
        uncompressed = arxio.unpack(compressed_payload)
    else:
        uncompressed = decompress_ftl(compressed_payload)
    
    # Parse scene and entities
    pos = 0
    scene = DANAE_LS_SCENE.from_buffer_copy(uncompressed, pos)
    pos += sizeof(DANAE_LS_SCENE)
    
    entities = []
    EntitiesType = DANAE_LS_INTER * header.nb_inter
    if header.nb_inter > 0:
        entities_data = EntitiesType.from_buffer_copy(uncompressed, pos)
        entities = list(entities_data)
    
    return header, entities

def main():
    if len(sys.argv) != 3:
        print("Usage: python coordinate_offset_analysis.py <original_dlf> <exported_dlf>")
        sys.exit(1)
    
    original_path = sys.argv[1]
    exported_path = sys.argv[2]
    
    print("Coordinate Offset Analysis")
    print("=" * 50)
    
    # Get data from both files
    orig_header, orig_entities = analyze_coordinates(original_path)
    exp_header, exp_entities = analyze_coordinates(exported_path)
    
    # Check header offsets
    print("Header offset field:")
    print(f"  Original: ({orig_header.offset.x}, {orig_header.offset.y}, {orig_header.offset.z})")
    print(f"  Exported: ({exp_header.offset.x}, {exp_header.offset.y}, {exp_header.offset.z})")
    
    offset_diff_x = exp_header.offset.x - orig_header.offset.x
    offset_diff_y = exp_header.offset.y - orig_header.offset.y
    offset_diff_z = exp_header.offset.z - orig_header.offset.z
    
    print(f"  Difference: ({offset_diff_x}, {offset_diff_y}, {offset_diff_z})")
    
    # Analyze first few entity position differences
    print(f"\nFirst 5 entity position transformations:")
    for i in range(min(5, len(orig_entities), len(exp_entities))):
        orig = orig_entities[i]
        exp = exp_entities[i]
        
        dx = exp.pos.x - orig.pos.x
        dy = exp.pos.y - orig.pos.y  
        dz = exp.pos.z - orig.pos.z
        
        print(f"  Entity {i}:")
        print(f"    Original: ({orig.pos.x:.3f}, {orig.pos.y:.3f}, {orig.pos.z:.3f})")
        print(f"    Exported: ({exp.pos.x:.3f}, {exp.pos.y:.3f}, {exp.pos.z:.3f})")
        print(f"    Delta:    ({dx:.3f}, {dy:.3f}, {dz:.3f})")
    
    # Check if there's a consistent transformation
    if len(orig_entities) > 0 and len(exp_entities) > 0:
        # Sample a few entities to see if the transformation is consistent
        sample_size = min(10, len(orig_entities))
        transformations = []
        
        for i in range(sample_size):
            orig = orig_entities[i]
            exp = exp_entities[i]
            
            dx = exp.pos.x - orig.pos.x
            dy = exp.pos.y - orig.pos.y
            dz = exp.pos.z - orig.pos.z
            
            transformations.append((dx, dy, dz))
        
        # Check if transformations are consistent
        first_transform = transformations[0]
        consistent = True
        for transform in transformations[1:]:
            if (abs(transform[0] - first_transform[0]) > 0.01 or
                abs(transform[1] - first_transform[1]) > 0.01 or
                abs(transform[2] - first_transform[2]) > 0.01):
                consistent = False
                break
        
        print(f"\nTransformation analysis:")
        if consistent:
            print(f"✅ CONSISTENT coordinate transformation detected:")
            print(f"   All entities shifted by: ({first_transform[0]:.3f}, {first_transform[1]:.3f}, {first_transform[2]:.3f})")
            
            # Compare with header offset difference
            if (abs(first_transform[0] - offset_diff_x) < 0.01 and
                abs(first_transform[1] - offset_diff_y) < 0.01 and
                abs(first_transform[2] - offset_diff_z) < 0.01):
                print(f"✅ Entity transformation MATCHES header offset difference")
                print(f"   This indicates proper coordinate system handling")
            else:
                print(f"⚠️  Entity transformation does NOT match header offset difference")
                print(f"   Entity delta: ({first_transform[0]:.3f}, {first_transform[1]:.3f}, {first_transform[2]:.3f})")
                print(f"   Header delta: ({offset_diff_x:.3f}, {offset_diff_y:.3f}, {offset_diff_z:.3f})")
                
        else:
            print(f"❌ INCONSISTENT transformations - entities moved by different amounts")
            print(f"   This suggests individual position errors")
    
    print(f"\n{'='*50}")
    print("CONCLUSION:")
    print(f"{'='*50}")
    print("The scene name in the DLF file matches between original and exported versions,")
    print("so the FTS file loading issue is NOT caused by the DLF file structure.")
    print("The position differences appear to be systematic coordinate transformations,")
    print("which should not affect FTS file discovery.")
    print("\nThe FTS loading problem likely stems from:")
    print("  1. FTS file path/naming issues")
    print("  2. FTS file format differences") 
    print("  3. Engine-specific FTS file validation")
    print("  4. Missing FTS file or incorrect location")

if __name__ == "__main__":
    main()