#!/usr/bin/env python3

import struct

def analyze_ftl_simple(filepath):
    """Simple FTL analysis focusing on groups"""
    with open(filepath, 'rb') as f:
        data = f.read()
    
    print(f"\n=== Analyzing {filepath} ===")
    print(f"File size: {len(data)} bytes")
    
    groups = []
    
    # Look for "all" group which should be near the end
    all_pos = data.find(b'all\x00')
    if all_pos >= 0:
        print(f"Found 'all' group at position: {all_pos}")
        
        # Look for origin: strings which indicate group names
        origin_positions = []
        search_pos = 0
        while True:
            pos = data.find(b'origin:', search_pos)
            if pos == -1:
                break
            origin_positions.append(pos)
            search_pos = pos + 1
        
        print(f"Found {len(origin_positions)} 'origin:' patterns")
        
        # Extract group names around origin: patterns
        for pos in origin_positions:
            # Try to extract the group name after origin:
            end_pos = data.find(b'\x00', pos)
            if end_pos > pos:
                name = data[pos:end_pos].decode('iso-8859-1', errors='ignore')
                groups.append((name, pos))
        
        print("Groups found:")
        for i, (name, pos) in enumerate(groups):
            print(f"  {i:2d}: {name}")
    
    return groups

def extract_group_structure(filepath):
    """Extract group structure and hierarchy info"""
    with open(filepath, 'rb') as f:
        data = f.read()
    
    print(f"\n=== Group Structure Analysis: {filepath} ===")
    
    # Find patterns that look like group definitions
    # Look for sequences that might be group headers
    
    # Search for numeric patterns that could be group indices
    groups_info = []
    
    # Find all occurrences of common group names
    group_names = [b'all', b'chest', b'neck', b'head', b'jaw', b'bum', b'tail']
    
    for name in group_names:
        pos = data.find(name + b'\x00')
        if pos >= 0:
            print(f"Found group '{name.decode()}' at position {pos}")
            
            # Look at the binary data around this position for numeric values
            start = max(0, pos - 20)
            end = min(len(data), pos + 60)
            
            print(f"  Hex dump around position:")
            for i in range(start, end, 16):
                chunk = data[i:i+16]
                hex_str = ' '.join(f'{b:02x}' for b in chunk)
                ascii_str = ''.join(chr(b) if 32 <= b <= 126 else '.' for b in chunk)
                print(f"    {i:06x}: {hex_str:<48} {ascii_str}")
    
    return groups_info

# Analyze both files
analyze_ftl_simple('dog_bak.ftl')
analyze_ftl_simple('dog.ftl')

extract_group_structure('dog_bak.ftl')
extract_group_structure('dog.ftl')