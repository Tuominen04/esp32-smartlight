# Usage: ./flash.ps1 [PORT]
param(
    [string]$Port = "COM3"  # Default to COM3; can override with -Port COMx
)

Write-Host "Setting up ESP-IDF environment..."
. "$env:IDF_PATH\export.ps1"  # Source the ESP-IDF environment

Write-Host "Fullclean and building the project..."
idf.py fullclean build

Write-Host "Erase flashing to device..."
idf.py -p $Port erase-flash

Write-Host "Flashing to device on port $Port..."
idf.py -p $Port flash monitor