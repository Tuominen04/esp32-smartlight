# ESP32 SmartLight Docker Flash Script (PowerShell)
# Flashes the firmware to ESP32 device via Docker

param(
    [string]$Port = "COM3",
    [string]$Baud = "460800"
)

$ErrorActionPreference = "Stop"

Write-Host "`n[*] Flashing ESP32 SmartLight firmware via Docker..." -ForegroundColor Cyan
Write-Host "[*] Port: $Port | Baud rate: $Baud`n" -ForegroundColor Cyan

# Check if docker is available
try {
    & docker compose version 2>&1 | Out-Null
} catch {
    Write-Host "[X] Error: Docker not found or docker compose not available." -ForegroundColor Red
    Write-Host "    Please install Docker Desktop for Windows." -ForegroundColor Red
    exit 1
}

# Check if build exists
if (-not (Test-Path "build\light_client.bin")) {
    Write-Host "[!] Build not found. Running build first..." -ForegroundColor Yellow
    & ".\docker_build.ps1"
    if ($LASTEXITCODE -ne 0) { exit 1 }
}

# Flash the device
Write-Host "[+] Flashing device..." -ForegroundColor Yellow
docker compose run --rm `
    -e PORT="$Port" `
    -e BAUD="$Baud" `
    esp32-flash bash -c ". /opt/esp/idf/export.sh && idf.py -p $Port -b $Baud flash"

if ($LASTEXITCODE -ne 0) { exit 1 }

Write-Host "`n[+] Flash complete! Device should now be running the new firmware.`n" -ForegroundColor Green
