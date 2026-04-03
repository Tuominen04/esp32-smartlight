# ESP32 Smart Light Project Guidelines

## Code Style

- **Language**: C with ESP-IDF framework (v5.0+)
- **Naming**: Snake_case for functions/variables, UPPER_SNAKE_CASE for macros/constants
- **Headers**: Include guards with `#ifndef`, `#define`, `#endif` pattern
- **Memory**: Use ESP_LOGI, ESP_LOGW, ESP_LOGE for logging; check return codes for all IDF calls
- **Error Handling**: Return esp_err_t or custom error types; propagate errors up

## Architecture

Components in `components/` are self-contained modules:
- Each component has its own `CMakeLists.txt` and public header
- BLE, WiFi, HTTP, OTA, and GPIO handle device communication and control
- Storage (NVS) and System Info provide persistent state and metadata
- Main entry point orchestrates component initialization

Refer to [README.md](../README.md) for system architecture diagram.

## Build and Test

Build: `idf.py set-target esp32c6 && idf.py build`
Flash: `idf.py -p <PORT> flash monitor`
Run tests: `idf.py build` then use `pytest` for unit tests in `test/`

ESP_LOGI tags: `BLUETOOTH`, `WIFI_MANAGER`, `HTTP_SERVER`, `OTA`

## Conventions

- Component headers declare public APIs; implementation is private
- Configuration via `sdkconfig.defaults` (board-specific) or menuconfig
- NVS key naming: `wifi_ssid`, `wifi_pass`, `device_id` (snake_case)
- HTTP endpoints documented in component READMEs
- OTA URLs must HTTPS; timeout is 60 seconds

