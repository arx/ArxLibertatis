#!/usr/bin/env python3

"""Test script to verify PKWare compression format"""

import sys
import os
sys.path.append('/home/burner/Desktop/ArxLibertatis/plugins/blender/arx_addon')

from dataFts import FtsSerializer
from lib import ArxIO
import logging

# Set up logging
logging.basicConfig(level=logging.DEBUG)

def test_compression():
    """Test the PKWare compression with a small data sample"""
    
    # Create serializer
    ioLib = ArxIO()
    serializer = FtsSerializer(ioLib)
    
    # Test with small data sample
    test_data = b"Hello World Test Data" * 100  # 2100 bytes
    print(f"Testing compression with {len(test_data)} bytes")
    
    # Test our PKWare implementation
    compressed = serializer._pkware_implode_fixed(test_data)
    print(f"Compressed to {len(compressed)} bytes")
    
    # Validate the output
    if serializer._validate_blast_compatibility(compressed):
        print("✓ Compression format validated successfully")
        
        # Write test file
        test_file = "/tmp/test_blast.dat"
        with open(test_file, "wb") as f:
            f.write(compressed)
        print(f"✓ Test file written to {test_file}")
        
        # Try to decompress with external tool if available
        # (This would need the actual C++ blast function)
        
    else:
        print("✗ Compression format validation failed")

if __name__ == "__main__":
    test_compression()