# ESP32 Smart Light Project Guidelines

## Overview

- **Language**: C with ESP-IDF framework (v5.0+), targeting ESP32-C6
- **Build**: `idf.py set-target esp32c6 && idf.py build`
- **Flash**: `idf.py -p <PORT> flash monitor`
- **Tests**: `pytest` for unit tests in `tests/`

## Architecture

Components in `components/` are self-contained modules (own `CMakeLists.txt` + public header).
Main entry point in `main/main.c` orchestrates initialization.
See [README.md](../README.md) for the system architecture diagram.

## Key Conventions

- Configuration via `sdkconfig.defaults` or menuconfig
- NVS keys: `wifi_ssid`, `wifi_pass`, `device_id` (snake_case)
- HTTP endpoints documented in component READMEs
- OTA URLs must use HTTPS; timeout is 60 seconds
- ESP_LOGI tags: `BLUETOOTH`, `WIFI_MANAGER`, `HTTP_SERVER`, `OTA`

## Detailed Rules

Code style, error handling, and security rules are in scoped instruction files:
- [C/H best practices](./instructions/c-h-best-practices.instructions.md) — naming, include guards, function size
- [ESP-IDF best practices](./instructions/esp-idf-best-practices.instructions.md) — error codes, event patterns, NVS
- [Security](./instructions/security.instructions.md) — secrets, PII, logging safety
- [Documentation](./instructions/documentation.instructions.md) — READMEs, component docs

