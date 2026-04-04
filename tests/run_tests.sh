#!/usr/bin/env bash
#
# Build and run ESP32 Smart Light tests
# Usage: ./run_tests.sh [unit|system|all] [--port PORT] [--target TARGET]

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"

# Default values
TEST_SUITE="all"
PORT=""
TARGET="esp32c6"

# Parse arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        unit|system|all)
            TEST_SUITE="$1"
            shift
            ;;
        --port)
            PORT="$2"
            shift 2
            ;;
        --target)
            TARGET="$2"
            shift 2
            ;;
        --help)
            echo "Usage: $0 [unit|system|all] [--port PORT] [--target TARGET]"
            echo ""
            echo "Arguments:"
            echo "  unit        Run unit tests only"
            echo "  system      Run system tests only"
            echo "  all         Run all tests (default)"
            echo "  --port      Serial port (e.g., /dev/ttyUSB0 or COM3)"
            echo "  --target    Target chip (default: esp32c6)"
            exit 0
            ;;
        *)
            echo "Unknown argument: $1"
            exit 1
            ;;
    esac
done

# Function to build tests
build_test() {
    local test_dir=$1
    local test_name=$2

    echo "========================================="
    echo "Building $test_name tests..."
    echo "========================================="

    cd "$test_dir"
    idf.py set-target "$TARGET"
    idf.py build
    cd "$PROJECT_ROOT"
}

# Function to flash and monitor tests
flash_and_monitor() {
    local test_dir=$1
    local test_name=$2

    echo "========================================="
    echo "Flashing and monitoring $test_name tests..."
    echo "========================================="

    cd "$test_dir"
    if [ -n "$PORT" ]; then
        idf.py -p "$PORT" flash monitor
    else
        idf.py flash monitor
    fi
    cd "$PROJECT_ROOT"
}

# Run tests based on selection
case $TEST_SUITE in
    unit)
        build_test "$SCRIPT_DIR/unit" "unit"
        if [ -n "$PORT" ] || [ -n "$ESPPORT" ]; then
            flash_and_monitor "$SCRIPT_DIR/unit" "unit"
        else
            echo "No port specified. Skipping flash and monitor."
            echo "Build artifacts are in: $SCRIPT_DIR/unit/build"
        fi
        ;;
    system)
        build_test "$SCRIPT_DIR/system" "system"
        if [ -n "$PORT" ] || [ -n "$ESPPORT" ]; then
            flash_and_monitor "$SCRIPT_DIR/system" "system"
        else
            echo "No port specified. Skipping flash and monitor."
            echo "Build artifacts are in: $SCRIPT_DIR/system/build"
        fi
        ;;
    all)
        build_test "$SCRIPT_DIR/unit" "unit"
        build_test "$SCRIPT_DIR/system" "system"

        if [ -n "$PORT" ] || [ -n "$ESPPORT" ]; then
            echo ""
            echo "========================================="
            echo "Both test suites built successfully!"
            echo "Flash them manually with:"
            echo "  cd $SCRIPT_DIR/unit && idf.py -p $PORT flash monitor"
            echo "  cd $SCRIPT_DIR/system && idf.py -p $PORT flash monitor"
            echo "========================================="
        else
            echo ""
            echo "========================================="
            echo "Both test suites built successfully!"
            echo "Build artifacts are in:"
            echo "  Unit: $SCRIPT_DIR/unit/build"
            echo "  System: $SCRIPT_DIR/system/build"
            echo "========================================="
        fi
        ;;
esac

echo ""
echo "Test execution complete!"
