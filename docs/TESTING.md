# Testing Guide

Short overview of the test setup for the ESP32 Smart Light project.

## Where Tests Live

```
tests/
├── unit/       # Unit tests — one component at a time
└── system/     # System tests — multiple components together
```

Both are independent ESP-IDF projects with their own `CMakeLists.txt` and
`sdkconfig`. They are completely isolated from the production firmware build.

## What Is Tested

| Suite | What it covers |
|-------|----------------|
| Unit | `device_info`, `gpio_control`, `nvs_manager`, `wifi_manager` |
| System | Boot sequence, component interaction, NVS read/write |

Tests use the [Unity](https://github.com/ThrowTheSwitch/Unity) framework that
ships with ESP-IDF.

## How to Run

### Prerequisites

- ESP-IDF v6.0.0 or later installed and sourced
- ESP32-C6 development board connected (for flash/monitor)

### Build and flash unit tests

```bash
cd tests/unit
idf.py set-target esp32c6
idf.py build
idf.py -p /dev/ttyUSB0 flash monitor
```

### Build and flash system tests

```bash
cd tests/system
idf.py set-target esp32c6
idf.py build
idf.py -p /dev/ttyUSB0 flash monitor
```

### Use the helper script

```bash
cd tests
./run_tests.sh unit   --target esp32c6 --port /dev/ttyUSB0
./run_tests.sh system --target esp32c6 --port /dev/ttyUSB0
./run_tests.sh all    --target esp32c6          # build only, no flash
```

## Adding a New Test

1. Create a `.c` file in `tests/unit/test_main/` (or `tests/system/test_main/`).
2. Write test functions using Unity macros:

```c
#include "unity.h"
#include "your_component.h"

void test_your_feature(void) {
    TEST_ASSERT_EQUAL(expected, actual);
}
```

3. Register the function in `test_main.c`:

```c
RUN_TEST(test_your_feature);
```

## Further Reading

- Full test infrastructure details: [`tests/README.md`](../tests/README.md)
- CI/CD pipeline: [`docs/CICD.md`](CICD.md)
- ESP-IDF unit testing guide: <https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-guides/unit-tests.html>
