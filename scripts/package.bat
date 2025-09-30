@echo off
setlocal

echo ====================================================
echo  Packaging BigBrother for Release
echo ====================================================
echo.

REM Check if build exists
if not exist "build\bin\Release\bigbro.exe" (
    echo ERROR: bigbro.exe not found. Build the project first!
    pause
    exit /b 1
)

if not exist "build\bin\Release\bigbrother_monitor.exe" (
    echo ERROR: bigbrother_monitor.exe not found. Build the project first!
    pause
    exit /b 1
)

REM Create release directory
if exist "release" rmdir /s /q release
mkdir release

echo Copying executables...
copy "build\bin\Release\bigbro.exe" "release\" >nul
copy "build\bin\Release\bigbrother_monitor.exe" "release\" >nul

echo Copying installation scripts...
copy "scripts\install.ps1" "release\" >nul
copy "scripts\uninstall.ps1" "release\" >nul

echo Creating README.txt...
copy "scripts\RELEASE_README.txt" "release\README.txt" >nul

REM Get version from VERSION file
set /p VERSION=<VERSION

REM Delete old ZIP if exists
if exist "bigbro-v%VERSION%.zip" (
    echo Removing old package...
    del "bigbro-v%VERSION%.zip"
)

echo Creating ZIP package...
powershell -Command "Add-Type -AssemblyName System.IO.Compression.FileSystem; [System.IO.Compression.ZipFile]::CreateFromDirectory('release', 'bigbro-v%VERSION%.zip')"

if exist "bigbro-v%VERSION%.zip" (
    echo.
    echo ====================================================
    echo  SUCCESS! Package created: bigbro-v%VERSION%.zip
    echo ====================================================
    echo.
    echo Contents:
    echo   - bigbro.exe
    echo   - bigbrother_monitor.exe
    echo   - install.ps1
    echo   - uninstall.ps1
    echo   - README.txt
    echo.
    echo Ready to upload to GitHub Releases!
) else (
    echo ERROR: Failed to create ZIP package
    exit /b 1
)

pause
