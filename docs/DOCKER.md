# Docker Development Guide

Complete guide for building and flashing ESP32 firmware using Docker.

## Quick Start

### VS Code DevContainer (Easiest)

1. Install [Docker Desktop](https://www.docker.com/products/docker-desktop) and [VS Code Remote Extension](https://marketplace.visualstudio.com/items?itemName=ms-vscode-remote.remote-containers)
2. Open project in VS Code → "Reopen in Container"
3. Build: `idf.py build` in terminal
4. Flash: `idf.py -p /dev/ttyUSB0 flash monitor` (use your port)

### Command Line

**Linux/macOS:**
```bash
./docker_build.sh          # Build
./docker_flash.sh /dev/ttyUSB0 460800  # Flash
```

**Windows (batch):**
```cmd
docker_build.bat           # Build
docker_flash.bat COM3      # Flash (default COM3)
```

**Windows (PowerShell):**
```powershell
.\docker_build.ps1                      # Build
.\docker_flash.ps1 -Port COM3 -Baud 460800  # Flash
```

## Find Your Device

| Platform | Examples |
|----------|----------|
| Linux | `/dev/ttyUSB0`, `/dev/ttyACM0` |
| macOS | `/dev/tty.usbserial-XXXXX` |
| Windows | `COM3`, `COM4` |

## Manual Docker Compose

```bash
# Build
docker compose run --rm esp32-build bash -c \
  '. /opt/esp/idf/export.sh && idf.py build'

# Flash
docker compose run --rm esp32-flash bash -c \
  '. /opt/esp/idf/export.sh && idf.py -p /dev/ttyUSB0 flash'

# Monitor
docker compose run --rm esp32-flash bash -c \
  '. /opt/esp/idf/export.sh && idf.py -p /dev/ttyUSB0 monitor'
```

## Troubleshooting

### Device Not Found
- **Linux:** Check `ls -la /dev/ttyUSB*`, then run `sudo usermod -a -G dialout $USER` (logout required)
- **macOS:** May need serial drivers (e.g., `brew install silicon-labs/silabs-software/vcp-driver`)
- **Windows:** Verify USB connection and port in Device Manager

### Docker Not Running
- **Windows:** Start Docker Desktop manually
- **macOS:** `open -a Docker`
- **Linux:** `sudo systemctl start docker`

### Out of Memory
- Docker Desktop → Settings → Resources → increase Memory slider

### Permission Denied on Scripts
```bash
chmod +x docker_build.sh docker_flash.sh
```

### Slow First Build
Normal—downloads ~2GB of ESP-IDF toolchain. Subsequent builds use cache.

## Environment Variables

```bash
export IDF_PATH=/custom/idf/path
export PORT=/dev/ttyUSB0
export BAUD=460800
```

## Remove DevContainer

```bash
docker compose down           # Stop containers
docker compose down -v        # Also remove volumes
docker compose down --rmi all # Also remove images
```

## Learn More

- [ESP-IDF Docs](https://docs.espressif.com/projects/esp-idf/)
- [Docker Docs](https://docs.docker.com/)
- [VS Code Remote Guide](https://code.visualstudio.com/docs/remote/remote-overview)
