| Supported Targets | ESP32 | ESP32-C2 | ESP32-C3 | ESP32-C5 | ESP32-C6 | ESP32-C61 | ESP32-H2 | ESP32-P4 | ESP32-S2 | ESP32-S3 | Linux |
| ----------------- | ----- | -------- | -------- | -------- | -------- | --------- | -------- | -------- | -------- | -------- | ----- |

# ESP32 Smart Light Starter

A comprehensive IoT light controller built on ESP-IDF that combines BLE setup, WiFi connectivity, HTTP control interface, and OTA firmware updates. Perfect for smart home applications and IoT learning projects.

## 🌟 Features

- BLE Configuration: Easy WiFi setup via Bluetooth Low Energy
- Remote Control: HTTP REST API for light control
- OTA Updates: Over-the-air firmware updates via HTTP
- Persistent Storage: WiFi credentials and device info stored in NVS
- Mobile App Ready: JSON-based communication for mobile integration
- Robust Architecture: Modular design with proper error handling
## 🏗️ System Architecture
```
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

## 🚀 Quick Start
Prerequisites

* ESP-IDF v5.0+ installed and configured
* ESP32 development board
* LED connected to GPIO 23 (configurable in `gpio_control.h`)
* Mobile device with BLE capability

Hardware Setup
`ESP32 GPIO 23 ──► LED ──► 220Ω Resistor ──► GND`

## How to get started with ESP-IDF

Select the instructions depending on Espressif chip installed on your development board:

- [ESP32 Getting Started Guide](https://docs.espressif.com/projects/esp-idf/en/stable/get-started/index.html)


## Installation

1. Clone and navigate to project
   ```bash
   git clone <repository-url>
   cd esp32-smartlight-starter
    ```

1. Set up ESP-IDF environment
    ```bash 
    # Windows (PowerShell)
    & "C:\Path\To\Your\esp-idf\install.ps1"
    & "C:\Path\To\Your\esp-idf\export.ps1"

    # Linux/Mac
    . $HOME/esp/esp-idf/export.sh
    ```

1. Configure and build
    ``` bash
    idf.py set-target esp32c6  # or your target chip
    idf.py build
    ```

1. Flash and monitor
    ```bash
    idf.py -p COM3 flash monitor  # Replace COM3 with your port
    ```

## 📁 Project Structure

```bash
esp32-smartlight-starter/
├── components/
│   ├── ble/                # BLE management
│   │   ├── ble_manager.c
│   │   └── ble_manager.h
│   ├── common/             # Shared definitions
│   │   └── common_defs.h
│   ├── gpio/               # GPIO and LED control
│   │   ├── gpio_control.c
│   │   └── gpio_control.h
│   ├── http/               # REST API server
│   │   ├── http_server.c
│   │   └──http_server.h
│   ├── ota/                # OTA update manager
│   │   ├── ota_manager.c
│   │   └── ota_manager.h
│   ├── storage/            # NVS storage manager
│   │   ├── nvs_manager.c
│   │   └── nvs_manager.h
│   ├── system_info/        # Device info
│   │   ├── device_info.c
│   │   └── device_info.h
│   └── wifi/               # WiFi connection
│       ├── wifi_manager.c
│       └── wifi_manager.h
├── docs/ 
│   ├── BLE.md
│   ...
├── licenses/               # Legal and licensing documents
│   ├── COPYRIGHT
│   └── THIRD_PARTY_LICENSES.md
├── main/                   # Entry point
│   ├── main.c
│   └── CMakeLists.txt
├── build_scripts/          # Build instructions
│   └── README_BUILD.md
├── test/                   # Unit and system tests
│   ├── system/
│   │   ├── test_main/
│   │   │   ├── CMakeList.txt
│   │   │   ├── test_main.c
│   │   │   └── test_system.c
│   │   └── CMakeList.txt
│   └── unit/
│       ├── test_main/
│       │   ├── CMakeList.txt
│       │   ├── test_device_info.c
│       │   ├── test_gpio_control.c
│       │   ├── test_main.c
│       │   ├── test_nvs_manager.c
│       │   └── test_wifi_manager.c
│       └── CMakeList.txt
├── CMakeLists.txt
├── sdkconfig.default
└── README.md
```

For more information on structure and contents of ESP-IDF projects, please refer to Section [Build System](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-guides/build-system.html) of the ESP-IDF Programming Guide.

## Troubleshooting
### Common Issues
#### BLE Connection Problems
- Ensure device is advertising (check logs)
- Verify BLE is enabled on mobile device
- Clear BLE cache if necessary

#### WiFi Issues
- Check credentials
- Ensure 2.4GHz band
- Review logs for errors

#### HTTP Server Inaccessibility
- Confirm IP address
- Same network between device and client
- Firewall settings

#### OTA Update Failures
- Firmware URL must be accessible
- Check flash size and compatibility

#### Log Analysis
Monitor serial output for detailed error information:
```bash
idf.py monitor
```
Key tags: ``BLUETOOTH``, ``WIFI_MANAGER``, ``HTTP_SERVER``, ``OTA``

## 📄 Legal Information

### License
This project is commercially licensed. See [LICENSE.md](/LICENSE.md) for full terms.

### Copyright
See [COPYRIGHT](licenses/COPYRIGHT) for ownership and rights.

### Third-Party Licenses
Open-source components listed in [THIRD_PARTY_LICENSES.md](licenses/THIRD_PARTY_LICENSES.md).

## Support
- GitHub Issues for bugs: [Project Issues](https://github.com/Tuominen04/esp32-smartlight-starter/issues)
- Email: arttutuominen10@gmail.com
- ESP-IDF docs and forums: [esp32.com](https://esp32.com/index.php)
