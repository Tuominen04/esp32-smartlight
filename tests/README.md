# Tests

Three independent test suites for the ESP32 Smart Light project.

## Suites

| Suite | Target | What it tests | Hardware needed |
| --- | --- | --- | --- |
| [unit/](unit/README.md) | linux | Constants, buffer limits, credential validation — pure logic only | No |
| [integration/](integration/README.md) | esp32c6 | Cross-component data flows: `device_info` ↔ `nvs_manager`, `wifi_manager` ↔ `nvs_manager` | Yes |
| [on_target/](on_target/README.md) | esp32c6 | Individual component behaviour on real hardware: GPIO, NVS, WiFi, device info | Yes |

All suites use the [Unity](https://github.com/ThrowTheSwitch/Unity) framework
that ships with ESP-IDF and are completely isolated from the production firmware
build.

## Quick Start

See each suite's README for full build and run instructions. Short version:

```bash
# Unit tests — runs in Docker, no device needed
docker run --rm -v "${PWD}:/project" espressif/idf:v6.0 bash -c \
  "rm -rf /project/tests/unit/build /project/tests/unit/sdkconfig* && \
   cd /project/tests/unit && \
   . /opt/esp/idf/export.sh && \
   idf.py --preview set-target linux && idf.py build && \
   ./build/unit_test_app.elf"

# Integration tests — requires ESP32-C6
cd tests/integration && idf.py set-target esp32c6 && idf.py build
idf.py -p /dev/ttyUSB0 flash monitor

# On-target tests — requires ESP32-C6
cd tests/on_target && idf.py set-target esp32c6 && idf.py build
idf.py -p /dev/ttyUSB0 flash monitor
```

## Test Structure

```txt
tests/
├── unit/           # linux target, runs in CI without hardware
├── integration/    # esp32c6, cross-component data flow tests
├── on_target/      # esp32c6, per-component hardware tests
├── system/         # esp32c6, boot and system-level tests
├── conftest.py
└── pytest.ini
```
