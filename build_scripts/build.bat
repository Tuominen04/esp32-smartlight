@echo off
echo Setting up ESP-IDF environment...

call %IDF_PATH%\export.bat

echo Cleaning and building the project...
idf.py clean build
