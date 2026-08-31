#!/usr/bin/env python3
"""
Test direct FTS read/write without Blender to isolate data corruption
"""

import sys
import os
sys.path.insert(0, os.path.dirname(__file__))

# Import without relative imports
import dataCommon
from dataCommon import SavedVec3, PolyTypeFlag
import lib
from lib import ArxIO

# Monkey patch the relative imports
sys.modules['dataFts.dataCommon'] = dataCommon
sys.modules['dataFts.lib'] = lib

from dataFts import FtsSerializer

def test_direct_roundtrip():
    print("=== Direct FTS Read/Write Test ===")
    
    # Initialize serializer
    ioLib = ArxIO()
    serializer = FtsSerializer(ioLib)
    
    # Read original file
    original_path = "/home/burner/Desktop/ArxLibertatis/Level one backup do not fucking touch/fast.fts"
    test_path = "/tmp/test_direct_roundtrip.fts"
    
    print(f"Reading original: {original_path}")
    header, fts_data = serializer.read_fts_container(original_path)
    print(f"Original header claims {header.uncompressedsize} bytes uncompressed")
    
    # Write back using exact same data (no Blender modification)
    print(f"Writing direct copy: {test_path}")
    serializer.write_fts_container(test_path, fts_data, updated_cells=None)
    
    print("=== Direct test completed ===")
    print("Now test the generated file with ArxLibertatis:")
    print(f"cp {test_path} {original_path}")
    print("./arx --loadlevel 01 -g --skiplogo --noclip")

if __name__ == "__main__":
    test_direct_roundtrip()