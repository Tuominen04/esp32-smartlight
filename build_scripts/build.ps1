Write-Host "Setting up ESP-IDF environment..."
. "$env:IDF_PATH\export.ps1"  # Use environment variable

Write-Host "Clean and building the project..."
idf.py clean build
