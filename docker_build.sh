#!/bin/bash

# ESP32 SmartLight Docker Build Script
# Builds the firmware inside a Docker container

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

echo "🐳 Building ESP32 SmartLight firmware in Docker..."

# Check if docker-compose is available
if ! command -v docker-compose &> /dev/null && ! command -v docker compose &> /dev/null; then
    echo "❌ Error: docker-compose not found. Please install Docker Compose."
    exit 1
fi

# Determine docker-compose command
if command -v docker-compose &> /dev/null; then
    COMPOSE_CMD="docker-compose"
else
    COMPOSE_CMD="docker compose"
fi

# Build the Docker image
echo "📦 Building Docker image..."
$COMPOSE_CMD build esp32-build

# Run the build
echo "🔨 Compiling firmware..."
$COMPOSE_CMD run --rm esp32-build bash -c \
    '. /opt/esp/idf/export.sh && idf.py build'

echo "✅ Build complete! Output: build/"
