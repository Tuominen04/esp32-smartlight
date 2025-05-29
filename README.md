| Supported Targets | ESP32 | ESP32-C2 | ESP32-C3 | ESP32-C5 | ESP32-C6 | ESP32-C61 | ESP32-H2 | ESP32-P4 | ESP32-S2 | ESP32-S3 | Linux |
| ----------------- | ----- | -------- | -------- | -------- | -------- | --------- | -------- | -------- | -------- | -------- | ----- |

# ESP32 Smart Light Controller

A comprehensive IoT light controller built on ESP-IDF that combines BLE setup, WiFi connectivity, HTTP control interface, and OTA firmware updates. Perfect for smart home applications and IoT learning projects.

## 🌟 Features

BLE Configuration: Easy WiFi setup via Bluetooth Low Energy
Remote Control: HTTP REST API for light control
OTA Updates: Over-the-air firmware updates via HTTP
Persistent Storage: WiFi credentials and device info stored in NVS
Mobile App Ready: JSON-based communication for mobile integration
Robust Architecture: Modular design with proper error handling

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
* LED connected to GPIO 23 (configurable in `gpio/gpio_control.h`)
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
   cd esp-idf-light-controller
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

```
esp-idf-light-controller/
├── main/
│   ├── ble/                 # BLE management and GATT server
│   │   ├── ble_manager.c
│   │   └── ble_manager.h
│   ├── common/              # Shared definitions and constants
│   │   └── common_defs.h
│   ├── device/              # Device info and identification
│   │   ├── device_info.c
│   │   └── device_info.h
│   ├── gpio/                # Hardware control (LED)
│   │   ├── gpio_control.c
│   │   └── gpio_control.h
│   ├── http/                # REST API server
│   │   ├── http_server.c
│   │   └── http_server.h
│   ├── ota/                 # Over-the-air updates
│   │   ├── ota_manager.c
│   │   └── ota_manager.h
│   ├── storage/             # NVS (non-volatile storage)
│   │   ├── nvs_manager.c
│   │   └── nvs_manager.h
│   ├── wifi/                # WiFi connection management
│   │   ├── wifi_manager.c
│   │   └── wifi_manager.h
│   ├── main.c               # Application entry point
│   └── CMakeLists.txt
├── CMakeLists.txt
└── README.md             *This is the file you are currently reading
```

For more information on structure and contents of ESP-IDF projects, please refer to Section [Build System](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-guides/build-system.html) of the ESP-IDF Programming Guide.

## 🐛 Troubleshooting
### Common Issues
#### BLE Connection Problems

* Ensure device is advertising (check logs for "Advertising started")
* Verify BLE is enabled on your mobile device
* Clear BLE cache on mobile device if connection fails

#### WiFi Connection Issues

* Check WiFi credentials are correct
* Verify network is 2.4GHz (most ESP32 don't support 5GHz)
* Monitor logs for specific error codes

#### HTTP Server Not Accessible

* Confirm device obtained IP address (check logs)
* Verify device and client are on same network
* Check firewall settings

#### OTA Update Problems

* Ensure firmware URL is accessible from device
* Verify sufficient flash space for update
* Check firmware is compatible with current chip

#### Log Analysis
Monitor serial output for detailed error information:
```bash
idf.py monitor
```
#### Key log tags to watch:
* ``BLUETOOTH``: BLE operations
* ``WIFI_MANAGER``: WiFi connection status
* ``HTTP_SERVER``: API requests
* ``OTA``: Update progress and errors

## 📞 Support
### IDF Problems: 
* Technical Issues: GitHub Issues
* ESP-IDF Documentation: Espressif Docs
* Community Forum: esp32.com

### Project specific problems:

Project Source: [GitHub Issues](https://github.com/Tuominen04/esp-idf-light-example/issues)