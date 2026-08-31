#!/usr/bin/env python3

import sys
sys.path.append('.')
from dataFtl import FtlSerializer
from dataCommon import *
import logging
logging.basicConfig(level=logging.INFO)

# Read both files
serializer = FtlSerializer()

with open('dog_bak.ftl', 'rb') as f:
    original_data = f.read()
original = serializer.read(original_data)

with open('dog.ftl', 'rb') as f:
    exported_data = f.read()
exported = serializer.read(exported_data)

print(f'Original: {len(original.groups)} groups')
print(f'Exported: {len(exported.groups)} groups')
print()

print('=== GROUP COMPARISON (first 10) ===')
for i in range(min(10, len(original.groups), len(exported.groups))):
    orig_grp = original.groups[i]
    exp_grp = exported.groups[i]
    
    print(f'Group {i}: {orig_grp.name} -> {exp_grp.name}')
    print(f'  Origin: {orig_grp.origin} -> {exp_grp.origin}')
    print(f'  Parent: {orig_grp.parentIndex} -> {exp_grp.parentIndex}')
    print(f'  Vertices: {len(orig_grp.indices)} -> {len(exp_grp.indices)}')
    if orig_grp.parentIndex != exp_grp.parentIndex:
        print('  *** PARENT MISMATCH ***')
    if orig_grp.origin != exp_grp.origin:
        print('  *** ORIGIN MISMATCH ***')
    print()

print('=== PARENT HIERARCHY COMPARISON ===')
print('Original hierarchy:')
for i, grp in enumerate(original.groups[:10]):
    parent_name = original.groups[grp.parentIndex].name if grp.parentIndex >= 0 else "ROOT"
    print(f'  {i:2d}: {grp.name:15} <- {parent_name}')

print('\nExported hierarchy:')
for i, grp in enumerate(exported.groups[:10]):
    parent_name = exported.groups[grp.parentIndex].name if grp.parentIndex >= 0 else "ROOT"
    print(f'  {i:2d}: {grp.name:15} <- {parent_name}')