@echo off
set PORT=COM3

if NOT "%1"=="" set PORT=%1

echo Setting up ESP-IDF environment...
call %IDF_PATH%\export.bat

echo Fullclean and building the project...
idf.py fullclean build

echo Erasing flash...
idf.py -p %PORT% erase-flash

echo Flashing to device on port %PORT%...
idf.py -p %PORT% flash monitor
