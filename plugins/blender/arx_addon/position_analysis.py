#!/usr/bin/env python3
"""
Position Precision Analysis - Check if position differences are due to precision issues
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

def analyze_positions(filepath):
    """Extract entity positions from DLF file"""
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
    
    return entities

def main():
    if len(sys.argv) != 3:
        print("Usage: python position_analysis.py <original_dlf> <exported_dlf>")
        sys.exit(1)
    
    original_path = sys.argv[1]
    exported_path = sys.argv[2]
    
    print("Position Precision Analysis")
    print("=" * 40)
    
    # Get entities from both files
    orig_entities = analyze_positions(original_path)
    exp_entities = analyze_positions(exported_path)
    
    if len(orig_entities) != len(exp_entities):
        print(f"❌ Different entity counts: {len(orig_entities)} vs {len(exp_entities)}")
        return
    
    print(f"Analyzing {len(orig_entities)} entities...")
    
    # Analyze position differences
    differences = []
    large_differences = []
    
    for i, (orig, exp) in enumerate(zip(orig_entities, exp_entities)):
        orig_name = orig.name[:512].decode('iso-8859-1', errors='ignore').rstrip('\x00')
        exp_name = exp.name[:512].decode('iso-8859-1', errors='ignore').rstrip('\x00')
        
        # Check name differences first
        if orig_name != exp_name:
            print(f"❌ Entity {i}: Name differs!")
            print(f"   Original: '{orig_name}'")
            print(f"   Exported: '{exp_name}'")
            continue
        
        # Calculate position differences
        dx = abs(orig.pos.x - exp.pos.x)
        dy = abs(orig.pos.y - exp.pos.y)
        dz = abs(orig.pos.z - exp.pos.z)
        max_diff = max(dx, dy, dz)
        
        differences.append(max_diff)
        
        if max_diff > 0.1:  # More than 0.1 unit difference
            large_differences.append((i, orig_name, max_diff, (dx, dy, dz)))
    
    # Statistics
    if differences:
        avg_diff = sum(differences) / len(differences)
        max_diff = max(differences)
        significant_diffs = sum(1 for d in differences if d > 0.001)
        
        print(f"\nPosition Difference Statistics:")
        print(f"  Average difference: {avg_diff:.6f} units")
        print(f"  Maximum difference: {max_diff:.6f} units")
        print(f"  Entities with >0.001 unit diff: {significant_diffs}/{len(differences)}")
        print(f"  Entities with >0.1 unit diff: {len(large_differences)}")
        
        if len(large_differences) > 0:
            print(f"\nLarge position differences (>0.1 units):")
            for i, name, max_diff, (dx, dy, dz) in large_differences[:10]:  # Show first 10
                print(f"  Entity {i}: {max_diff:.3f} units - dx:{dx:.3f}, dy:{dy:.3f}, dz:{dz:.3f}")
                if len(name) > 60:
                    print(f"    {name[:60]}...")
                else:
                    print(f"    {name}")
            
            if len(large_differences) > 10:
                print(f"    ... and {len(large_differences) - 10} more")
        
        # Check if differences look like precision issues
        precision_issues = sum(1 for d in differences if 0 < d < 0.001)
        if precision_issues > len(differences) * 0.8:
            print(f"\n✅ Most differences appear to be precision-related (<0.001 units)")
            print(f"   This should NOT affect FTS file loading.")
        elif len(large_differences) > 0:
            print(f"\n⚠️  Some entities have significant position differences")
            print(f"   This might indicate export issues but shouldn't affect FTS loading.")
        
    else:
        print("✅ All positions are identical!")

if __name__ == "__main__":
    main()