# Build & Flash Script Guide
This project includes cross-platform scripts to simplify building and flashing the ESP-IDF firmware. These scripts handle environment setup and common actions like clean builds and flashing.

## 🔧 Available Scripts
| Script Type | File(s)              | OS Compatibility     |
|-------------|----------------------|----------------------|
| Bash Shell  | build.sh, flash.sh   | Linux / macOS        |
| PowerShell  | build.ps1, flash.ps1 | Windows (PowerShell) |
| CMD Batch	  | build.bat, flash.bat | Windows (CMD)        |

## 🧪 Usage Instructions
### 🐧 Linux / macOS (bash)
```bash
chmod +x build.sh flash.sh
./build.sh
./flash.sh /dev/ttyUSB0
```

### 🪟 Windows PowerShell
```ps
cd /path/to/project/esp-idf-light-example
./build_scripts/build.ps1
./build_scripts/flash.ps1 -Port COM5
```
### 🪟 Windows CMD
To build and flash the project using Command Prompt (`cmd.exe`):
```cmd
:: If your project is on the C: drive:
cd /path/to/project/esp-idf-light-example
build_scripts/build.bat
build_scripts/flash.bat COM5
```

#### *If your project is on another drive (e.g., D:)*
You must switch drives manually before running the script:
```cmd
:: Switch to the correct drive
D:

:: Navigate to the project folder
cd \ESP\ESP-SDK\light_bulb\esp-idf-light-example

:: Run build and flash scripts
D:./build_scripts\build.bat
D:./build_scripts\flash.bat COM5
```

## ℹ️ Notes
- All scripts assume that the `IDF_PATH` environment variable is correctly set.

- Scripts automatically source the ESP-IDF environment (`export.sh`, `export.ps1`, or `export.bat`).

- `flash` scripts will erase flash and monitor the device after upload.

- If no port is specified, the default is used:
  - `/dev/ttyUSB0` on Linux/macOS
  - `COM3` on Windows

Feel free to modify these scripts to match your board type, serial port, or custom flashing options.