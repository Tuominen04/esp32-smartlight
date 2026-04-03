#!/bin/bash

# ESP32 SmartLight Docker Flash Script
# Flashes the firmware to ESP32 device via Docker

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

# Parse arguments
PORT=${1:-/dev/ttyUSB0}
BAUD=${2:-460800}

echo "🐳 Flashing ESP32 SmartLight firmware via Docker..."
echo "📍 Port: $PORT | Baud rate: $BAUD"

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

# Check if build exists
if [ ! -f "build/light_client.bin" ]; then
    echo "⚠️  Build not found. Running build first..."
    bash "$SCRIPT_DIR/docker_build.sh"
fi

# Flash the device
echo "⚡ Flashing device..."
$COMPOSE_CMD run --rm \
    -e PORT="$PORT" \
    -e BAUD="$BAUD" \
    esp32-flash bash -c \
    '. /opt/esp/idf/export.sh && idf.py -p '"$PORT"' -b '"$BAUD"' flash'

echo "✅ Flash complete! Device should now be running the new firmware."
