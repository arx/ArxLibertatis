#!/usr/bin/env python3

import sys
from struct import unpack

def analyze_ftl_groups(filepath):
    """Analyze FTL file groups without importing modules"""
    with open(filepath, 'rb') as f:
        data = f.read()
    
    print(f"File: {filepath}")
    print(f"Size: {len(data)} bytes")
    
    if len(data) < 520:  # Minimum size for headers
        print("ERROR: File too small")
        return
    
    # Skip primary header and checksum
    pos = 4 + 4 + 512  # ident + version + checksum
    
    # Read secondary header
    offset_3Ddata, = unpack('<l', data[pos:pos+4])
    pos = offset_3Ddata
    
    # Read 3D data header
    nb_vertex, nb_faces, nb_maps, nb_groups, nb_action, nb_selections, origin = unpack('<lllllll', data[pos:pos+28])
    pos += 28 + 256  # Skip name field
    
    print(f"Groups: {nb_groups}")
    
    # Skip vertices and faces
    pos += nb_vertex * 56  # EERIE_OLD_VERTEX size
    pos += nb_faces * 136  # EERIE_FACE_FTL size  
    pos += nb_maps * 256   # Texture_Container_FTL size
    
    # Read groups
    groups = []
    for i in range(nb_groups):
        name = data[pos:pos+256].split(b'\x00')[0].decode('iso-8859-1')
        group_origin, nb_index = unpack('<ll', data[pos+256:pos+264])
        pos += 256 + 4 + 4 + 4 + 4  # name + origin + nb_index + indexes + siz
        groups.append((name, group_origin, nb_index))
    
    # Read group indices
    group_indices = []
    for name, origin, count in groups:
        indices = []
        for _ in range(count):
            idx, = unpack('<l', data[pos:pos+4])
            pos += 4
            indices.append(idx)
        group_indices.append(indices)
    
    return groups, group_indices

def calculate_parent_indices(groups, group_indices):
    """Calculate parent indices like the original algorithm"""
    parent_indices = []
    for child_idx, (child_name, child_origin, _) in enumerate(groups):
        parent_idx = -1
        # Search previous groups
        for i in range(child_idx - 1, -1, -1):
            _, _, _ = groups[i]
            if child_origin in group_indices[i]:
                parent_idx = i
                break
        parent_indices.append(parent_idx)
    return parent_indices

# Analyze both files
print("=== ORIGINAL FILE ===")
orig_groups, orig_indices = analyze_ftl_groups('dog_bak.ftl')
orig_parents = calculate_parent_indices(orig_groups, orig_indices)

print("\n=== EXPORTED FILE ===")
exp_groups, exp_indices = analyze_ftl_groups('dog.ftl')
exp_parents = calculate_parent_indices(exp_groups, exp_indices)

print("\n=== COMPARISON ===")
for i in range(min(len(orig_groups), len(exp_groups))):
    orig_name, orig_origin, _ = orig_groups[i]
    exp_name, exp_origin, _ = exp_groups[i]
    
    print(f"Group {i}: {orig_name}")
    print(f"  Origin: {orig_origin} -> {exp_origin}")
    print(f"  Parent: {orig_parents[i]} -> {exp_parents[i]}")
    
    if orig_parents[i] != exp_parents[i]:
        print("  *** PARENT MISMATCH ***")
    if orig_origin != exp_origin:
        print("  *** ORIGIN MISMATCH ***")