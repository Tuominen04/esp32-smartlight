# ESP32 Smart Light

| Supported Targets | ESP32 | ESP32-C2 | ESP32-C3 | ESP32-C5 | ESP32-C6 | ESP32-C61 | ESP32-H2 | ESP32-P4 | ESP32-S2 | ESP32-S3 | Linux |
| ----------------- | ----- | -------- | -------- | -------- | -------- | --------- | -------- | -------- | -------- | -------- | ----- |

A full-stack IoT light controller built with ESP-IDF (C, ESP32-C6). It combines BLE-based WiFi
provisioning, an HTTP REST API, and OTA firmware updates in a modular component architecture.

## Project Highlights

*For those reviewing this as a portfolio piece.*

### What is demonstrated

- Embedded C following ESP-IDF v5 conventions — event loops, NVS, `esp_err_t` error handling
- Modular component design: each feature lives in `components/` with its own public header
- BLE provisioning flow: mobile app sends WiFi credentials over GATT, device connects and
  stores them in NVS
- HTTP REST API served from the device itself — control GPIO and query status over the local
  network
- HTTPS OTA via `esp_https_ota` with a 60-second timeout and verified server certificate
- Unity-based unit and system tests in `tests/`, runnable via `pytest` on host or on-target
- CI/CD pipeline in `.github/workflows/` — build and test on every push

### Architecture

```text
┌─────────────────┐    ┌──────────────────┐    ┌─────────────────┐
│   Mobile App    │◄──►│  ESP32 Device    │◄──►│  WiFi Network   │
│  (BLE Client)   │    │                  │    │                 │
└─────────────────┘    │  ┌─────────────┐ │    └─────────────────┘
                       │  │ BLE Manager │ │              │
                       │  └─────────────┘ │              │
                       │  ┌─────────────┐ │              ▼
                       │  │WiFi Manager │ │    ┌─────────────────┐
                       │  └─────────────┘ │    │  HTTP Client    │
                       │  ┌─────────────┐ │    │ (OTA Updates)   │
                       │  │HTTP Server  │ │    └─────────────────┘
                       │  └─────────────┘ │
                       │  ┌─────────────┐ │
                       │  │GPIO Control │ │
                       │  └─────────────┘ │
                       └──────────────────┘
```

### Key source files

| File | Purpose |
| ---- | ------- |
| [main/main.c](main/main.c) | Entry point — initialises all components in order |
| [components/ble/ble_manager.c](components/ble/ble_manager.c) | BLE GATT server and provisioning logic |
| [components/wifi/wifi_manager.c](components/wifi/wifi_manager.c) | WiFi connect / reconnect with event handling |
| [components/http/http_server.c](components/http/http_server.c) | REST API handlers |
| [components/ota/ota_manager.c](components/ota/ota_manager.c) | HTTPS OTA update flow |
| [components/storage/nvs_manager.c](components/storage/nvs_manager.c) | NVS read/write wrappers |

### Documentation

Component-level API docs, configuration keys, and error codes are in [docs/](docs/).

## Quick Start

*For developers who want to use or extend this project.*

### Prerequisites

- ESP-IDF v5.0+ installed and configured
- ESP32 development board (primary target: ESP32-C6)
- LED connected to GPIO 23 (configurable in `gpio_control.h`)
- Mobile device with BLE capability for provisioning

**Hardware wiring:**

```text
ESP32 GPIO 23 ──► LED ──► 220Ω Resistor ──► GND
```

### Installation

1. Clone and navigate to the project

   ```bash
   git clone <repository-url>
   cd esp32-smartlight
   ```

1. Set up the ESP-IDF environment

   ```bash
   # Windows (PowerShell)
   & "C:\Path\To\Your\esp-idf\install.ps1"
   & "C:\Path\To\Your\esp-idf\export.ps1"

   # Linux / macOS
   . $HOME/esp/esp-idf/export.sh
   ```

1. Configure and build

   ```bash
   idf.py set-target esp32c6  # or your target chip
   idf.py build
   ```

1. Flash and monitor

   ```bash
   idf.py -p COM3 flash monitor  # replace COM3 with your port
   ```

For more detail on ESP-IDF projects see the
[Build System guide](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-guides/build-system.html).

## Project Structure

```text
esp32-smartlight/
├── components/
│   ├── ble/                # BLE management
│   ├── common/             # Shared definitions
│   ├── gpio/               # GPIO and LED control
│   ├── http/               # REST API server
│   ├── ota/                # OTA update manager
│   ├── storage/            # NVS storage manager
│   ├── system_info/        # Device info
│   └── wifi/               # WiFi connection
├── docs/                   # Component API and feature docs
├── licenses/               # Legal and licensing documents
├── main/                   # Entry point (main.c)
├── tests/                  # Unit and system tests
│   ├── unit/
│   └── system/
├── CMakeLists.txt
├── sdkconfig.defaults
└── README.md
```

## Testing

Tests use the Unity framework and can run on-target or via `pytest` on host.

### Test types

- Unit tests — `tests/unit/`, test individual components in isolation
- Integraion tests — `tests/integraion/`, test integration and full device behaviour
- On-target tests — `tests/on_target/`, require physical hardware

### Running tests

```bash
# Integraion tests
cd tests/integraion
idf.py set-target esp32c6
idf.py build
idf.py -p /dev/ttyUSB0 flash monitor

# On-target tests
cd tests/on_target
idf.py set-target esp32c6
idf.py build
idf.py -p /dev/ttyUSB0 flash monitor
```

For full details see [tests/README.md](tests/README.md).

## Troubleshooting

### BLE connection problems

- Confirm the device is advertising (check serial logs for `BLUETOOTH` tag)
- Verify BLE is enabled on the mobile device
- Clear BLE cache on the mobile device and retry

### WiFi issues

- Double-check credentials and confirm the network is 2.4 GHz
- Review logs tagged `WIFI_MANAGER` for error codes

### HTTP server not reachable

- Confirm the device IP from logs and that the client is on the same network
- Check local firewall rules

### OTA update failures

- The firmware URL must be HTTPS and reachable from the device network
- Verify flash partition sizes match the new firmware

### Log monitoring

```bash
idf.py monitor
```

Key log tags: `BLUETOOTH`, `WIFI_MANAGER`, `HTTP_SERVER`, `OTA`

## Legal Information

### License

This project is commercially licensed. See [LICENSE.md](LICENSE.md) for full terms.

### Copyright

See [licenses/COPYRIGHT](licenses/COPYRIGHT) for ownership and rights.

### Third-party licenses

Open-source components listed in [licenses/THIRD_PARTY_LICENSES.md](licenses/THIRD_PARTY_LICENSES.md).

## Support

- Bug reports and feature requests: [GitHub Issues](https://github.com/Tuominen04/esp32-smartlight/issues)
- Email: <arttutuominen10@gmail.com>
- ESP-IDF documentation and community: [esp32.com](https://esp32.com/index.php)
