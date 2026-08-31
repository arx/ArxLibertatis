#!/bin/bash

# Script to run FTL round trip tests in headless Blender
# Usage: ./run_ftl_roundtrip_test.sh

BLENDER_PATH="/home/burner/blender-4.4.3-linux-x64/blender"
ARX_DATA_DIR="/home/burner/.steam/debian-installation/steamapps/common/Arx Fatalis"
PLUGIN_DIR="/home/burner/Desktop/ArxLibertatis/plugins/blender"

# Change to the plugin directory
cd "$PLUGIN_DIR"

# Run Blender in headless mode with the test script
echo "Running FTL round trip tests..."
echo "Blender path: $BLENDER_PATH"
echo "Arx data dir: $ARX_DATA_DIR"
echo "Plugin dir: $PLUGIN_DIR"

# Execute the test
"$BLENDER_PATH" -b --python test_roundtrip.py -- -d "$ARX_DATA_DIR"

echo "Test completed. Check test_files.txt and test_errors.txt for results."