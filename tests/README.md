# ESP32 Smart Light - Testing Guide

This directory contains the test infrastructure for the ESP32 Smart Light project, including unit tests and system integration tests.

## Test Structure

```txt
tests/
├── unit/                   # Unit tests for individual components
│   ├── CMakeLists.txt     # Independent ESP-IDF project configuration
│   ├── sdkconfig          # Unit test-specific SDK configuration
│   └── test_main/         # Test component
│       ├── CMakeLists.txt # Test component registration
│       ├── test_main.c    # Test runner
│       ├── test_device_info.c
│       ├── test_gpio_control.c
│       ├── test_nvs_manager.c
│       └── test_wifi_manager.c
│
├── system/                 # System integration tests
│   ├── CMakeLists.txt     # Independent ESP-IDF project configuration
│   ├── sdkconfig          # System test-specific SDK configuration
│   └── test_main/         # Test component
│       ├── CMakeLists.txt # Test component registration
│       ├── test_main.c    # Test runner
│       └── test_system.c  # System integration tests
│
├── conftest.py            # Shared pytest fixtures
├── pytest.ini             # Pytest configuration
├── run_tests.sh           # Test execution script
└── README.md              # This file
```

## Prerequisites

1. **ESP-IDF v5.0+** installed and configured
2. **Python 3.8+** with pip
3. **pytest** (optional, for automated test execution)
4. **pytest-embedded** (optional, for hardware-in-the-loop testing)

### Installing Python Dependencies

```bash
pip install pytest pytest-embedded pytest-embedded-serial-esp pytest-embedded-idf
```

## Building Tests

### Build Isolation

The test projects are completely isolated from the main firmware build:

- **Production build**: `idf.py build` in project root (excludes all tests)
- **Unit tests**: Independent ESP-IDF project in `tests/unit/`
- **System tests**: Independent ESP-IDF project in `tests/system/`

### Building Unit Tests

```bash
cd tests/unit
idf.py set-target esp32c6  # or your target chip
idf.py build
```

### Building System Tests

```bash
cd tests/system
idf.py set-target esp32c6  # or your target chip
idf.py build
```

### Building All Tests

Use the provided test runner script:

```bash
cd tests
./run_tests.sh all --target esp32c6
```

## Running Tests

### Manual Execution

#### Unit Tests

```bash
cd tests/unit
idf.py set-target esp32c6
idf.py build
idf.py -p /dev/ttyUSB0 flash monitor  # Replace with your port
```

#### System Tests

```bash
cd tests/system
idf.py set-target esp32c6
idf.py build
idf.py -p /dev/ttyUSB0 flash monitor  # Replace with your port
```

### Using Test Runner Script

The `run_tests.sh` script provides a convenient way to build and optionally flash tests:

```bash
# Build and flash unit tests
./run_tests.sh unit --port /dev/ttyUSB0 --target esp32c6

# Build and flash system tests
./run_tests.sh system --port /dev/ttyUSB0 --target esp32c6

# Build all tests (no flash)
./run_tests.sh all --target esp32c6
```

### Using Pytest (Future)

When pytest-embedded is fully integrated:

```bash
# Run unit tests only
pytest tests/unit -m unit

# Run system tests only
pytest tests/system -m system

# Run all tests
pytest tests/
```

## Test Configuration

### Unity Test Framework

Both unit and system tests use the Unity test framework from ESP-IDF. Tests are registered using standard Unity patterns:

```c
void app_main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_function_name);
    UNITY_END();
}
```

### Deliberate Failure Tests

Tests that are expected to fail (for framework validation) are gated behind the `CONFIG_TEST_DELIBERATE_FAIL` Kconfig flag. By default, these tests are **not** executed in CI or normal test runs.

To enable deliberate failure tests:

```bash
cd tests/system  # or tests/unit
idf.py menuconfig
# Navigate to Component config → Test Configuration
# Enable "Include deliberate failure tests"
```

## Command Matrix

| Command | Description | Location |
| --- | --- | --- |
| `cd tests/unit && idf.py build` | Build unit tests only | `tests/unit/` |
| `cd tests/system && idf.py build` | Build system tests only | `tests/system/` |
| `./run_tests.sh unit` | Build unit tests | `tests/` |
| `./run_tests.sh system` | Build system tests | `tests/` |
| `./run_tests.sh all` | Build all tests | `tests/` |
| `./run_tests.sh unit --port PORT` | Build and flash unit tests | `tests/` |
| `pytest tests/unit` | Run unit tests with pytest (future) | `tests/` |
| `pytest tests/system` | Run system tests with pytest (future) | `tests/` |
| `pytest tests/` | Run all tests with pytest (future) | `tests/` |

## CI Integration

Tests can be executed in GitHub Actions workflows using the provided test scripts. See `.github/workflows/test.yml` for the automated test execution pipeline.

The CI workflow:

1. Builds unit and system test firmware
2. Collects build artifacts
3. Optionally flashes and executes tests on hardware
4. Generates JUnit XML reports
5. Archives test results

## Component Architecture

Tests use proper ESP-IDF component linkage instead of direct source file inclusion:

- Each component in `components/` has its own `CMakeLists.txt`
- Tests declare component dependencies via `REQUIRES` and `PRIV_REQUIRES`
- No manual `include_directories` for component paths needed
- Production components are reused in tests without duplication

## Troubleshooting

### Build Errors

**Problem**: Component not found during test build

```txt
CMake Error: Could not find component 'gpio'
```

**Solution**: Ensure the test project's CMakeLists.txt includes the root components directory:

```cmake
set(EXTRA_COMPONENT_DIRS
    "$ENV{IDF_PATH}/tools/unit-test-app/components"
    "../../components"
    "test_main"
)
```

### Flash Errors

**Problem**: Tests fail to flash

```cmd
Failed to connect to ESP32
```

**Solution**:

1. Check serial port connection
2. Verify correct permissions: `sudo usermod -a -G dialout $USER` (Linux)
3. Try resetting the device
4. Check correct target chip is set: `idf.py set-target esp32c6`

### Test Hangs

**Problem**: Test runner hangs indefinitely

**Solution**: Ensure test runner returns from `app_main()` after `UNITY_END()`. Do not use infinite loops like `while(1)` in test code.

## Writing New Tests

### Unit Test Example

Create a new test file in `tests/unit/test_main/`:

```c
#include "unity.h"
#include "your_component.h"

void test_your_feature(void) {
    TEST_ASSERT_EQUAL(expected, actual);
}

// Add to test_main.c:
// RUN_TEST(test_your_feature);
```

### System Test Example

Create integration tests in `tests/system/test_main/test_system.c`:

```c
#include "unity.h"
#include "esp_system.h"

void test_system_integration(void) {
    // Test multiple components working together
    TEST_ASSERT_NOT_NULL(component_init());
}
```

## Best Practices

1. **Isolation**: Keep unit tests focused on single components
2. **Independence**: Tests should not depend on execution order
3. **Cleanup**: Always clean up resources in tearDown()
4. **Assertions**: Use appropriate Unity assertion macros
5. **Naming**: Use descriptive test names: `test_component_feature_scenario()`
6. **Documentation**: Document complex test setups and expected behavior

## Additional Resources

- [ESP-IDF Testing Guide](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-guides/unit-tests.html)
- [Unity Test Framework](https://github.com/ThrowTheSwitch/Unity)
- [pytest Documentation](https://docs.pytest.org/)
- [pytest-embedded Documentation](https://docs.espressif.com/projects/pytest-embedded/en/latest/)
