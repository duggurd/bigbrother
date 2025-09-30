@echo off
echo ====================================================
echo  BigBrother Project Setup
echo ====================================================
echo.

REM Move to project root
cd /d "%~dp0\.."

echo Setting up directory structure...

REM Ensure all directories exist
if not exist src\common mkdir src\common
if not exist src\monitor mkdir src\monitor
if not exist src\viewer\ui mkdir src\viewer\ui
if not exist src\viewer\data mkdir src\viewer\data
if not exist src\viewer\graphics mkdir src\viewer\graphics
if not exist third_party mkdir third_party
if not exist docs mkdir docs
if not exist scripts mkdir scripts
if not exist build mkdir build

echo.
echo Directory structure created!
echo.
echo Next steps:
echo 1. Run scripts\setup_imgui.bat to download Dear ImGui
echo 2. Run scripts\build_cmake.bat to build the project
echo.
pause
