#!/usr/bin/env python3

import sys
sys.path.append('.')
from dataFtl import FtlSerializer, getFatherIndex
import logging

def analyze_correct_ordering():
    """Analyze what the correct group ordering should be"""
    serializer = FtlSerializer()
    
    with open('dog.ftl', 'rb') as f:
        data = f.read()
    
    ftl_data = serializer.read(data)
    
    print("=== CURRENT WRONG ORDERING ===")
    for i, group in enumerate(ftl_data.groups):
        print(f"Group {i:2d}: {group.name:15} origin={group.origin:3d}")
    
    print("\n=== ANALYSIS: What should 'all' contain as children? ===")
    all_group = ftl_data.groups[23]  # Current 'all' group
    print(f"'all' vertices: {all_group.indices}")
    
    print("\nWhich groups have their origin vertex in 'all's vertex list?")
    potential_children = []
    for i, group in enumerate(ftl_data.groups):
        if group.origin in all_group.indices:
            potential_children.append((i, group.name, group.origin))
            print(f"  Group {i:2d}: {group.name:15} (origin {group.origin}) <- should be child of 'all'")
    
    print(f"\nFound {len(potential_children)} potential children of 'all'")
    
    print("\n=== TESTING MANUAL REORDERING ===")
    print("Let's try putting 'all' first and see what happens...")
    
    # Create a new ordering with 'all' first
    new_groups = [ftl_data.groups[23]]  # Start with 'all'
    remaining_groups = [g for i, g in enumerate(ftl_data.groups) if i != 23]
    new_groups.extend(remaining_groups)
    
    print("New ordering:")
    for i, group in enumerate(new_groups):
        print(f"Group {i:2d}: {group.name:15} origin={group.origin:3d}")
    
    print("\n=== Testing getFatherIndex with new ordering ===")
    for i, group in enumerate(new_groups):
        father = getFatherIndex(new_groups, i)
        parent_name = "ROOT" if father < 0 else new_groups[father].name
        print(f"Group {i:2d}: {group.name:15} calculated father: {father} [{parent_name}]")
    
    print("\n=== BETTER APPROACH: Order by dependency ===")
    print("The groups should be ordered so parents come before children.")
    print("Let's build a proper hierarchy:")
    
    # Find all origin vertices that appear in other groups' vertex lists
    vertex_to_groups = {}  # vertex -> list of groups that contain it
    origin_to_group = {}   # origin vertex -> group that has it as origin
    
    for i, group in enumerate(ftl_data.groups):
        origin_to_group[group.origin] = i
        for vertex in group.indices:
            if vertex not in vertex_to_groups:
                vertex_to_groups[vertex] = []
            vertex_to_groups[vertex].append(i)
    
    print("\nVertex sharing analysis:")
    parent_child_relationships = []
    for origin_vertex, child_group_idx in origin_to_group.items():
        child_group = ftl_data.groups[child_group_idx]
        if origin_vertex in vertex_to_groups:
            # This group's origin vertex appears in other groups
            containing_groups = [g for g in vertex_to_groups[origin_vertex] if g != child_group_idx]
            for parent_group_idx in containing_groups:
                parent_group = ftl_data.groups[parent_group_idx]
                parent_child_relationships.append((parent_group_idx, parent_group.name, child_group_idx, child_group.name, origin_vertex))
                print(f"  {parent_group.name} contains vertex {origin_vertex} which is origin of {child_group.name}")
    
    print(f"\nFound {len(parent_child_relationships)} parent-child relationships:")
    for parent_idx, parent_name, child_idx, child_name, vertex in parent_child_relationships:
        print(f"  {parent_name} -> {child_name} (via vertex {vertex})")

if __name__ == "__main__":
    analyze_correct_ordering()