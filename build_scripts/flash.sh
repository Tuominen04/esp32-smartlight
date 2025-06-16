#!/bin/bash

# Usage: ./flash.sh [PORT]
PORT=${1:-/dev/ttyUSB0}  # Default port if none is provided

echo "Setting up ESP-IDF environment..."

# Source the ESP-IDF environment using IDF_PATH
source "$IDF_PATH/export.sh"

echo "Fullclean and building the project..."
idf.py fullclean build

echo "Erasing flash on device..."
idf.py -p "$PORT" erase-flash

echo "Flashing to device on port $PORT..."
idf.py -p "$PORT" flash monitor
