@echo off
echo ====================================================
echo  Building BigBrother with CMake
echo ====================================================
echo.

REM Check for CMake
where cmake >nul 2>nul
if %ERRORLEVEL% NEQ 0 (
    echo ERROR: CMake not found!
    echo Please install CMake from https://cmake.org/download/
    pause
    exit /b 1
)

REM Create build directory if it doesn't exist
if not exist build mkdir build

REM Navigate to build directory
cd build

REM Configure with CMake
echo Configuring...
cmake -G "Visual Studio 17 2022" -A x64 ..
if %ERRORLEVEL% NEQ 0 (
    echo.
    echo CMake configuration failed!
    echo.
    echo If you don't have Visual Studio 2022, you can try:
    echo   cmake -G "Visual Studio 16 2019" -A x64 ..
    echo or for MinGW:
    echo   cmake -G "MinGW Makefiles" ..
    cd ..
    pause
    exit /b 1
)

REM Build
echo.
echo Building...
cmake --build . --config Release
if %ERRORLEVEL% NEQ 0 (
    echo Build failed!
    cd ..
    pause
    exit /b 1
)

echo.
echo ====================================================
echo  Build successful!
echo ====================================================
echo.
echo Executables are in: build\bin\Release\
echo   - bigbrother_monitor.exe
echo   - bigbrother_viewer.exe (if ImGui is set up)
echo.

cd ..
pause
