# Integration Tests

Tests that verify **two or more components working together** on real
ESP32-C6 hardware. They exercise cross-component data flows that unit
tests cannot cover in isolation.

## What Is Tested

| File | What it covers |
| --- | --- |
| `test_device_nvs_integration.c` | `device_info` + `nvs_manager`: saving device info generates a correctly formatted name and ID that can be read back from NVS |
| `test_wifi_nvs_integration.c` | `wifi_manager` + `nvs_manager`: credentials written via `nvs_manager` are returned by `wifi_manager_get_saved_credentials`; absent credentials return a non-OK error code |

## Prerequisites

- ESP-IDF v6.0 installed and sourced
- ESP32-C6 development board connected via USB

## Building

```bash
cd tests/integration
idf.py set-target esp32c6
idf.py build
```

## Flashing and Running

```bash
idf.py -p /dev/ttyUSB0 flash monitor
```

Replace `/dev/ttyUSB0` with your actual serial port.

Expected output ends with:

```txt
7 Tests 0 Failures 0 Ignored
OK
```

## Adding a New Test

1. Create `tests/integration/test_main/test_<a>_<b>_integration.c` where `<a>`
   and `<b>` are the two components under test.
1. Write test functions that exercise the real interaction:

    ```c
    #include "unity.h"
    #include "component_a.h"
    #include "component_b.h"

    void test_a_writes_b_reads(void)
    {
      TEST_ASSERT_EQUAL(ESP_OK, component_a_save("value"));
      char buf[32] = {0};
      TEST_ASSERT_EQUAL(ESP_OK, component_b_load(buf, sizeof(buf)));
      TEST_ASSERT_EQUAL_STRING("value", buf);
    }
    ```

1. Declare `extern` and call `RUN_TEST` in `test_main.c`.
1. Add the `.c` file and component dependencies to `test_main/CMakeLists.txt`.
