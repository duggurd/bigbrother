@echo off
pushd "%~dp0"
echo ==========================================
echo    BigBrother Installer Build Script
echo ==========================================

echo.
echo [1/2] Building Executable...
call build_exe.bat

echo.
echo [2/2] Building Installer...
echo Checking for Inno Setup Compiler (ISCC.exe)...

set ISCC="C:\Program Files (x86)\Inno Setup 6\ISCC.exe"

if not exist %ISCC% (
    echo.
    echo ERROR: Inno Setup 6 not found at default location:
    echo %ISCC%
    echo.
    echo Please install Inno Setup 6 from: https://jrsoftware.org/isdl.php
    echo.
    pause
    popd
    exit /b 1
)

%ISCC% scripts\installer.iss

echo.
echo ==========================================
echo    Installer Built Successfully!
echo    Location: scripts\Output\BigBrother_Setup.exe
echo ==========================================
popd
pause
