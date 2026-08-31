#!/usr/bin/env python3

import sys
sys.path.append('.')
from dataFtl import FtlSerializer, getFatherIndex
import logging
logging.basicConfig(level=logging.DEBUG)

def analyze_ftl_hierarchy(filepath):
    """Analyze FTL hierarchy with detailed debugging"""
    serializer = FtlSerializer()
    
    with open(filepath, 'rb') as f:
        data = f.read()
    
    print(f"\n=== Analyzing {filepath} ===")
    
    try:
        ftl_data = serializer.read(data)
        
        print(f"Successfully loaded FTL with {len(ftl_data.groups)} groups")
        
        print("\n=== Group Details ===")
        for i, group in enumerate(ftl_data.groups):
            parent_name = "ROOT" if group.parentIndex < 0 else ftl_data.groups[group.parentIndex].name
            print(f"Group {i:2d}: {group.name:15} (origin: {group.origin:3d}, parent: {group.parentIndex:2d} [{parent_name}], vertices: {len(group.indices)})")
            
            # Show the first few vertex indices to understand the structure
            if len(group.indices) > 0:
                indices_preview = group.indices[:5]
                if len(group.indices) > 5:
                    indices_preview.append("...")
                print(f"          Vertices: {indices_preview}")
        
        print("\n=== Hierarchy Tree ===")
        
        def print_tree(group_idx, level=0):
            if group_idx < 0 or group_idx >= len(ftl_data.groups):
                return
            
            group = ftl_data.groups[group_idx]
            indent = "  " * level
            print(f"{indent}{group.name} (origin: {group.origin}, vertices: {len(group.indices)})")
            
            # Find children
            children = [i for i, g in enumerate(ftl_data.groups) if g.parentIndex == group_idx]
            for child_idx in children:
                print_tree(child_idx, level + 1)
        
        # Find root groups (those with parentIndex -1)
        root_groups = [i for i, g in enumerate(ftl_data.groups) if g.parentIndex == -1]
        print(f"Found {len(root_groups)} root groups")
        
        for root_idx in root_groups:
            print_tree(root_idx)
        
        return ftl_data
        
    except Exception as e:
        print(f"Error loading {filepath}: {e}")
        import traceback
        traceback.print_exc()
        return None

def compare_hierarchies():
    """Compare hierarchies between original and exported"""
    print("="*60)
    print("HIERARCHY COMPARISON")
    print("="*60)
    
    original = analyze_ftl_hierarchy('dog_bak.ftl')
    exported = analyze_ftl_hierarchy('dog.ftl')
    
    if original and exported:
        print("\n=== COMPARISON SUMMARY ===")
        print(f"Original groups: {len(original.groups)}")
        print(f"Exported groups: {len(exported.groups)}")
        
        # Check if group names match
        if len(original.groups) == len(exported.groups):
            print("\n=== Group-by-Group Comparison ===")
            for i in range(len(original.groups)):
                orig = original.groups[i]
                exp = exported.groups[i]
                
                if orig.name != exp.name or orig.parentIndex != exp.parentIndex:
                    print(f"DIFF Group {i}: {orig.name} -> {exp.name}")
                    print(f"     Parent: {orig.parentIndex} -> {exp.parentIndex}")
                    if orig.parentIndex >= 0 and orig.parentIndex < len(original.groups):
                        orig_parent = original.groups[orig.parentIndex].name
                    else:
                        orig_parent = "ROOT"
                    
                    if exp.parentIndex >= 0 and exp.parentIndex < len(exported.groups):
                        exp_parent = exported.groups[exp.parentIndex].name
                    else:
                        exp_parent = "ROOT"
                    
                    print(f"     Parent names: {orig_parent} -> {exp_parent}")

if __name__ == "__main__":
    compare_hierarchies()