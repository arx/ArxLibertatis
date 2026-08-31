#!/bin/bash

# TEA Round Trip Test Script for Blender 4.4.3

BLENDER_PATH="/home/burner/blender-4.4.3-linux-x64/blender"
SCRIPT_PATH="/home/burner/Desktop/ArxLibertatis/plugins/blender/test_tea_roundtrip.py"
GAME_DIR="/home/burner/Desktop/ArxLibertatis"

# Check if arguments were provided
if [ $# -eq 0 ]; then
    echo "Usage: $0 <tea_file> [model_file]"
    echo "Example: $0 /path/to/animation.tea /path/to/model.ftl"
    exit 1
fi

TEA_FILE="$1"
MODEL_FILE="$2"

# Check if TEA file exists
if [ ! -f "$TEA_FILE" ]; then
    echo "Error: TEA file '$TEA_FILE' not found"
    exit 1
fi

# Check if model file exists (if provided)
if [ -n "$MODEL_FILE" ] && [ ! -f "$MODEL_FILE" ]; then
    echo "Error: Model file '$MODEL_FILE' not found"
    exit 1
fi

# Build command and run
echo "Running TEA round trip test..."
echo "TEA file: $TEA_FILE"
if [ -n "$MODEL_FILE" ]; then
    echo "Model file: $MODEL_FILE"
fi
echo ""

# Run the test with proper quoting
if [ -n "$MODEL_FILE" ]; then
    "$BLENDER_PATH" --background --python "$SCRIPT_PATH" -- "$TEA_FILE" --model "$MODEL_FILE" --game-dir "$GAME_DIR"
else
    "$BLENDER_PATH" --background --python "$SCRIPT_PATH" -- "$TEA_FILE" --game-dir "$GAME_DIR"
fi