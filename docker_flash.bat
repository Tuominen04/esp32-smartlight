@echo off
REM ESP32 SmartLight Docker Flash Script (Windows)
REM Flashes the firmware to ESP32 device via Docker

setlocal enabledelayedexpansion

set PORT=%1
set BAUD=%2

if "!PORT!"=="" set PORT=COM3
if "!BAUD!"=="" set BAUD=460800

echo.
echo [*] Flashing ESP32 SmartLight firmware via Docker...
echo [*] Port: !PORT! ^| Baud rate: !BAUD!
echo.

REM Check if docker compose is available
docker compose version >nul 2>&1
if errorlevel 1 (
    echo [X] Error: Docker not found or docker compose not available.
    echo     Please install Docker Desktop for Windows.
    exit /b 1
)

REM Check if build exists
if not exist "build\light_client.bin" (
    echo [!] Build not found. Running build first...
    call docker_build.bat
    if errorlevel 1 exit /b 1
)

REM Flash the device
echo [+] Flashing device...
docker compose run --rm ^
    -e PORT="!PORT!" ^
    -e BAUD="!BAUD!" ^
    esp32-flash bash -c ". /opt/esp/idf/export.sh && idf.py -p !PORT! -b !BAUD! flash"

if errorlevel 1 exit /b 1

echo.
echo [+] Flash complete! Device should now be running the new firmware.
echo.
