# On-Target Tests

Tests that run directly on the **ESP32-C6** hardware. They verify component
behaviour using real peripherals, NVS flash, WiFi stack, and GPIO.

## What Is Tested

| File | What it covers |
| --- | --- |
| `test_device_info.c` | Firmware info retrieval, BLE handle/info setting (ignored when BLE disabled), invalid parameter handling |
| `test_gpio_control.c` | GPIO initialisation, light state control, toggle, rapid state changes, timing |
| `test_nvs_manager.c` | NVS init, WiFi credential save/load/delete, device info save/load, invalid params, buffer sizes |
| `test_wifi_manager.c` | WiFi manager init, connection state, credential handling, JSON format, disconnect, callbacks |

## Prerequisites

- ESP-IDF v6.0 installed and sourced
- ESP32-C6 development board connected via USB

## Building

```bash
cd tests/on_target
idf.py set-target esp32c6
idf.py build
```

## Flashing and Running

```bash
idf.py -p /dev/ttyUSB0 flash monitor
```

Replace `/dev/ttyUSB0` with your actual serial port (`COM3`, `/dev/cu.usbserial-*`, etc.).

The monitor output ends with a Unity summary, for example:

```txt
27 Tests 0 Failures 2 Ignored
OK
```

Ignored tests are BLE-specific tests that are skipped when `CONFIG_BT_ENABLED` is not
set in the sdkconfig.

## Adding a New Test

1. Create `tests/on_target/test_main/test_<component>.c`.
1. Write test functions:

    ```c
    #include "unity.h"
    #include "your_component.h"

    void test_your_feature(void)
    {
      TEST_ASSERT_EQUAL(ESP_OK, your_component_init());
    }
    ```

1. Declare `extern` and call `RUN_TEST` in `test_main.c`.
1. Add the `.c` file and any new component dependencies to `test_main/CMakeLists.txt`.
