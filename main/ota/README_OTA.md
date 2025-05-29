# OTA (Over-The-Air) Update Guide
This guide explains how to create firmware binary files and set up a local server for OTA updates with the ESP32 Smart Light Controller.

## 🔍 Overview
The OTA system allows you to update your ESP32 firmware remotely without physical access to the device. The process involves:

1. Building a new firmware binary (.bin file)
1. Hosting the binary on an HTTP server
1. Triggering the update via the device's HTTP API
1. Device downloads and installs the new firmware automatically

## 🔨 Creating Firmware Binary Files
Using ESP-IDF Build System
Step 1: Modify Your Code
Make changes to your project code (e.g., update version number, add features, fix bugs).
1. Modify main project or create a copy of the project and modify that. 

   ```c
   // In your main/copy application, update version in CMakeLists.txt
   project(light_client VERSION 0.0.2)  // Increment version
   ```
1. Build the Project
   ```bash
   # Navigate to your project directory
   cd C:/path/to/your/project/esp-idf-light-controller

   # Clean previous build (recommended)
   idf.py fullclean

   # Build the project
   idf.py build
   ```
1. Locate the Binary File
After successful build, the binary files are located in:
   ```
   build/
   ├── light_client.bin          # Main application binary (use this for OTA)
   ├── bootloader/
   │   └── bootloader.bin        # Bootloader (don't use for OTA)
   └── partition_table/
      └── partition-table.bin    # Partition table (don't use for OTA)
   ```
**Important**: Use only light_client.bin (or your project name) for OTA updates.

## 🌐 Setting Up Local HTTP Server
### Simple HTTP Server
   ```bash
   # Navigate to directory containing your .bin file
   cd C:/path/to/your/project/esp-idf-light-controller/build

   # Start Python HTTP server
   # Python 3
   python -m http.server 8080
   ```
Your firmware will be available at: ``http://your-computer-ip:8080/light_client.bin```

## 🚀 Performing OTA Updates
1. Find Your Computer's IP Address
   ```bash
   # Windows
   ipconfig

   # Linux/Mac
   ifconfig
   # or
   ip addr show
   ```
1. Verify Server Accessibility <br>
   Test if your firmware is accessible:
   ```
   bash# Test with curl
   curl -I http://your-computer-ip:8080/light_client.bin

   # Or open in browser
   http://your-computer-ip:8080/light_client.bin
   ```
1. Trigger OTA Update
   Using curl
   ```bash 
   curl -X POST http://device-ip/ota/update \
   -H "Content-Type: application/json" \
   -d '{"url":"http://your-computer-ip:8080/light_client.bin"}'
   ```
1. Monitor Update Progress
   ```bash 
   # Check progress
   curl http://device-ip/ota/progress

   # Example response
   {
    "in_progress": true,
    "progress": 45.2,
    "status": "Downloading..."
   }
   ```

# 🐛 Troubleshooting
### Common Issues
1. **"Failed to connect to update server"**

* Cause: Server not accessible from ESP32
* Solution:
  * Check if server is running: ``netstat -an | grep :8080``
  * Verify ESP32 and server are on same network
  * Test with ``curl`` from another device on the network



2. **"Invalid firmware image"**

* Cause: Wrong binary file or corrupted download
* Solution:
  * Verify you're using the correct .bin file (app binary, not bootloader)
  * Check file integrity with checksums
  * Ensure firmware is built for the same chip type



3. **"Not enough space for OTA update"**

* Cause: Insufficient flash space
* Solution:
  * Check partition table: idf.py partition-table
  * Reduce firmware size or increase OTA partition size
  * Use idf.py size to analyze binary size



4. **"OTA update validation failed"**

* Cause: Firmware signature/checksum mismatch
* Solution:
  * Rebuild firmware completely (idf.py fullclean && idf.py build)
  * Check if secure boot is enabled (if so, sign the firmware)
  * Verify download completed successfully

## OTA Status Codes
| Code                          | Meaning          | Action |
| -------------                 | -------------    | ------------- |
| `ESP_OK`                      | Success          | Update completed
| `ESP_ERR_OTA_VALIDATE_FAILED` | Invalid firmware | Check binary file
| `ESP_ERR_NO_MEM`              | Out of memory    | Reduce firmware size
| `ESP_ERR_FLASH_OP_FAIL`       | Flash write error| Check hardware
| `ESP_ERR_TIMEOUT`             | Download timeout | Check network/server

## 🔗 Useful Resources

* [ESP-IDF Build System](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-guides/build-system.html)
* [Partition Tables](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-guides/partition-tables.html)
