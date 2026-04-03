# ESP32 SmartLight Docker Build Script (PowerShell)
# Builds the firmware inside a Docker container

$ErrorActionPreference = "Stop"

Write-Host "`n[*] Building ESP32 SmartLight firmware in Docker...`n" -ForegroundColor Cyan

# Check if docker is available
try {
    & docker compose version 2>&1 | Out-Null
} catch {
    Write-Host "[X] Error: Docker not found or docker compose not available." -ForegroundColor Red
    Write-Host "    Please install Docker Desktop for Windows." -ForegroundColor Red
    exit 1
}

# Build the Docker image
Write-Host "[+] Building Docker image..." -ForegroundColor Yellow
docker compose build esp32-build
if ($LASTEXITCODE -ne 0) { exit 1 }

# Run the build
Write-Host "[+] Compiling firmware..." -ForegroundColor Yellow
docker compose run --rm esp32-build bash -c ". /opt/esp/idf/export.sh && idf.py build"
if ($LASTEXITCODE -ne 0) { exit 1 }

Write-Host "`n[+] Build complete! Output: build/`n" -ForegroundColor Green
