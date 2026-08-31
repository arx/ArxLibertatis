#!/usr/bin/env python3

import sys
sys.path.append('.')
from dataFtl import FtlSerializer, getFatherIndex
import logging

def debug_group_creation():
    """Debug the current exported file to show the group hierarchy calculation"""
    serializer = FtlSerializer()
    
    with open('dog.ftl', 'rb') as f:
        data = f.read()
    
    print("=== DEBUGGING CURRENT EXPORTED FILE ===")
    
    try:
        ftl_data = serializer.read(data)
        
        print(f"Loaded {len(ftl_data.groups)} groups")
        
        print("\n=== Groups BEFORE getFatherIndex calculation ===")
        for i, group in enumerate(ftl_data.groups):
            print(f"Group {i:2d}: {group.name:15} origin={group.origin:3d} parent={group.parentIndex:2d} vertices={len(group.indices)}")
        
        print("\n=== Applying getFatherIndex algorithm ===")
        # Apply the same algorithm that should be used during export
        for i, group in enumerate(ftl_data.groups):
            father = getFatherIndex(ftl_data.groups, i)
            print(f"Group {i:2d}: {group.name:15} calculated father: {father}")
            group.parentIndex = father
        
        print("\n=== Groups AFTER getFatherIndex calculation ===")
        for i, group in enumerate(ftl_data.groups):
            parent_name = "ROOT" if group.parentIndex < 0 else ftl_data.groups[group.parentIndex].name
            print(f"Group {i:2d}: {group.name:15} origin={group.origin:3d} parent={group.parentIndex:2d} [{parent_name}] vertices={len(group.indices)}")
        
        print("\n=== Hierarchy Tree ===")
        
        def print_tree(group_idx, level=0):
            if group_idx < 0 or group_idx >= len(ftl_data.groups):
                return
            
            group = ftl_data.groups[group_idx]
            indent = "  " * level
            print(f"{indent}{group.name} (origin: {group.origin})")
            
            # Find children
            children = [i for i, g in enumerate(ftl_data.groups) if g.parentIndex == group_idx]
            for child_idx in sorted(children):
                print_tree(child_idx, level + 1)
        
        # Find root groups (those with parentIndex -1)
        root_groups = [i for i, g in enumerate(ftl_data.groups) if g.parentIndex == -1]
        print(f"Found {len(root_groups)} root groups")
        
        for root_idx in sorted(root_groups):
            print_tree(root_idx)
        
        # Check the specific case mentioned by user: "00:all" should have children "01" and "14"
        print("\n=== Checking User's Specific Feedback ===")
        if len(ftl_data.groups) > 0:
            all_group = ftl_data.groups[23]  # "all" was at index 23 in our previous output
            print(f"Group 'all' is at index 23: {all_group.name}")
            
            # Look for groups that should be children of "all"
            children_of_all = [i for i, g in enumerate(ftl_data.groups) if g.parentIndex == 23]
            print(f"Children of 'all' (index 23): {children_of_all}")
            
            # Check chest (01) and bum (14)
            chest_group = ftl_data.groups[22]  # "chest" was at index 22
            bum_group = ftl_data.groups[9]     # "bum" was at index 9
            print(f"Chest (index 22): {chest_group.name}, parent: {chest_group.parentIndex}")
            print(f"Bum (index 9): {bum_group.name}, parent: {bum_group.parentIndex}")
            
            # User says the original has "00:all" with children "01" and "14"
            # Let's check what vertices are in each group to understand the relationships
            print(f"\nVertex analysis:")
            print(f"'all' vertices: {all_group.indices}")
            print(f"'chest' vertices: {chest_group.indices}")
            print(f"'bum' vertices: {bum_group.indices}")
            
            # Check if chest's origin vertex is in all's vertex list
            print(f"\nChecking if chest origin {chest_group.origin} is in all's vertices: {chest_group.origin in all_group.indices}")
            print(f"Checking if bum origin {bum_group.origin} is in all's vertices: {bum_group.origin in all_group.indices}")
        
    except Exception as e:
        print(f"Error: {e}")
        import traceback
        traceback.print_exc()

if __name__ == "__main__":
    debug_group_creation()