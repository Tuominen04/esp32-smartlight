@echo off
REM ESP32 SmartLight Docker Build Script (Windows)
REM Builds the firmware inside a Docker container

setlocal enabledelayedexpansion

echo.
echo [*] Building ESP32 SmartLight firmware in Docker...
echo.

REM Check if docker compose is available
docker compose version >nul 2>&1
if errorlevel 1 (
    echo [X] Error: Docker not found or docker compose not available.
    echo     Please install Docker Desktop for Windows.
    exit /b 1
)

REM Build the Docker image
echo [+] Building Docker image...
docker compose build esp32-build
if errorlevel 1 exit /b 1

REM Run the build
echo [+] Compiling firmware...
docker compose run --rm esp32-build bash -c ". /opt/esp/idf/export.sh && idf.py build"
if errorlevel 1 exit /b 1

echo.
echo [+] Build complete! Output: build/
echo.
