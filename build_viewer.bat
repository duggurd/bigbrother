@echo off
setlocal enabledelayedexpansion
echo Building BigBrother Session Viewer with ImGui...
echo.

REM Check if imgui directory exists
if not exist "imgui\" (
    echo ERROR: ImGui library not found!
    echo.
    echo Please download Dear ImGui from https://github.com/ocornut/imgui
    echo Extract it and place the following files in an 'imgui' folder:
    echo   - imgui.cpp
    echo   - imgui_demo.cpp
    echo   - imgui_draw.cpp
    echo   - imgui_tables.cpp
    echo   - imgui_widgets.cpp
    echo   - imgui.h
    echo   - imconfig.h
    echo   - imgui_internal.h
    echo   - imstb_rectpack.h
    echo   - imstb_textedit.h
    echo   - imstb_truetype.h
    echo   - backends\imgui_impl_win32.cpp
    echo   - backends\imgui_impl_win32.h
    echo   - backends\imgui_impl_dx11.cpp
    echo   - backends\imgui_impl_dx11.h
    echo.
    pause
    exit /b 1
)

REM Try Visual Studio first (cl.exe)
REM Check if we're already in a Visual Studio environment
where cl >nul 2>nul
if %ERRORLEVEL% EQU 0 (
    REM Check if environment is properly set up by testing for standard includes
    echo Testing Visual Studio environment...
    echo #include ^<float.h^> > test_compile.cpp
    echo int main^(^) ^{ return 0; ^} >> test_compile.cpp
    cl /nologo test_compile.cpp >nul 2>nul
    set VS_TEST=!ERRORLEVEL!
    del test_compile.cpp test_compile.obj test_compile.exe 2>nul
    
    if !VS_TEST! EQU 0 (
        echo Using Visual Studio compiler...
        cl /EHsc /I. /Iimgui /Iimgui\backends ^
            focus_log_viewer.cpp ^
            imgui\imgui.cpp ^
            imgui\imgui_demo.cpp ^
            imgui\imgui_draw.cpp ^
            imgui\imgui_tables.cpp ^
            imgui\imgui_widgets.cpp ^
            imgui\backends\imgui_impl_win32.cpp ^
            imgui\backends\imgui_impl_dx11.cpp ^
            /link user32.lib gdi32.lib shell32.lib d3d11.lib d3dcompiler.lib dxgi.lib ole32.lib ^
            /OUT:focus_log_viewer.exe
        
        if %ERRORLEVEL% EQU 0 (
            echo.
            echo Build successful! Run with: focus_log_viewer.exe
            goto :end
        ) else (
            echo Visual Studio build failed!
            goto :try_mingw
        )
    ) else (
        echo Visual Studio found but environment not properly initialized.
        echo Please run this script from "Developer Command Prompt for VS" or try MinGW.
        goto :try_mingw
    )
) else (
    echo Visual Studio compiler not found in PATH.
)

:try_mingw
echo Trying MinGW...

where g++ >nul 2>nul
if %ERRORLEVEL% EQU 0 (
    echo Using MinGW compiler...
    g++ -o focus_log_viewer.exe ^
        focus_log_viewer.cpp ^
        imgui\imgui.cpp ^
        imgui\imgui_demo.cpp ^
        imgui\imgui_draw.cpp ^
        imgui\imgui_tables.cpp ^
        imgui\imgui_widgets.cpp ^
        imgui\backends\imgui_impl_win32.cpp ^
        imgui\backends\imgui_impl_dx11.cpp ^
        -I. -Iimgui -Iimgui\backends ^
        -luser32 -lgdi32 -lshell32 -ld3d11 -ld3dcompiler -ldxgi -lole32
    
    if %ERRORLEVEL% EQU 0 (
        echo.
        echo Build successful! Run with: focus_log_viewer.exe
        goto :end
    ) else (
        echo MinGW build failed!
    )
) else (
    echo.
    echo ==============================================================================
    echo No suitable compiler found or properly configured!
    echo.
    echo For Visual Studio:
    echo   - Open "Developer Command Prompt for VS" from Start Menu
    echo   - Navigate to this directory and run build_viewer.bat again
    echo.
    echo For MinGW:
    echo   - Install MinGW-w64 from https://www.mingw-w64.org/
    echo   - Add MinGW bin directory to your PATH
    echo ==============================================================================
)

:end
pause

