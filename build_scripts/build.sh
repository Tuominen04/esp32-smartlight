#!/bin/bash

echo "Setting up ESP-IDF environment..."

# Use the IDF_PATH environment variable to source ESP-IDF
source "$IDF_PATH/export.sh"

echo "Clean and building the project..."
idf.py clean build
