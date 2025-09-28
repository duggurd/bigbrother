@echo off
echo Building Window Focus Monitor...

REM Try Visual Studio first (cl.exe)
where cl >nul 2>nul
if %ERRORLEVEL% EQU 0 (
    echo Using Visual Studio compiler...
    cl /EHsc window_focus_monitor.cpp user32.lib psapi.lib
    if %ERRORLEVEL% EQU 0 (
        echo Build successful! Run with: window_focus_monitor.exe
        goto :end
    ) else (
        echo Visual Studio build failed!
    )
) else (
    echo Visual Studio compiler not found, trying MinGW...
)

REM Try MinGW as fallback
where g++ >nul 2>nul
if %ERRORLEVEL% EQU 0 (
    echo Using MinGW compiler...
    g++ -o window_focus_monitor.exe window_focus_monitor.cpp -luser32 -lpsapi
    if %ERRORLEVEL% EQU 0 (
        echo Build successful! Run with: window_focus_monitor.exe
        goto :end
    ) else (
        echo MinGW build failed!
    )
) else (
    echo No suitable compiler found! Please install Visual Studio Build Tools or MinGW.
)

:end
pause
