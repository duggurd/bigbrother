@echo off
setlocal enabledelayedexpansion
echo ====================================================
echo  Building Refactored Viewer (Without CMake)
echo ====================================================
echo.

REM Check for ImGui
if not exist "third_party\imgui\imgui.cpp" (
    echo ERROR: ImGui not found!
    echo Please run scripts\setup_imgui.bat first
    pause
    exit /b 1
)

REM Try Visual Studio
where cl >nul 2>nul
if %ERRORLEVEL% EQU 0 (
    echo Testing Visual Studio environment...
    echo #include ^<float.h^> > test_compile.cpp
    echo int main^(^) ^{ return 0; ^} >> test_compile.cpp
    cl /nologo test_compile.cpp >nul 2>nul
    set VS_TEST=!ERRORLEVEL!
    del test_compile.cpp test_compile.obj test_compile.exe 2>nul
    
    if !VS_TEST! EQU 0 (
        echo Using Visual Studio compiler...
        cl /EHsc /std:c++17 ^
            /Isrc\common /Isrc\viewer /Ithird_party /Ithird_party\imgui /Ithird_party\imgui\backends ^
            src\viewer\main.cpp ^
            src\viewer\ui\main_window.cpp ^
            src\viewer\ui\settings_window.cpp ^
            src\viewer\ui\timeline_view.cpp ^
            src\viewer\data\session_loader.cpp ^
            src\viewer\data\filter_manager.cpp ^
            src\viewer\graphics\icon_manager.cpp ^
            third_party\imgui\imgui.cpp ^
            third_party\imgui\imgui_demo.cpp ^
            third_party\imgui\imgui_draw.cpp ^
            third_party\imgui\imgui_tables.cpp ^
            third_party\imgui\imgui_widgets.cpp ^
            third_party\imgui\backends\imgui_impl_win32.cpp ^
            third_party\imgui\backends\imgui_impl_dx11.cpp ^
            /link user32.lib gdi32.lib shell32.lib psapi.lib d3d11.lib d3dcompiler.lib dxgi.lib ole32.lib ^
            /OUT:bigbrother_viewer_new.exe
        
        if !ERRORLEVEL! EQU 0 (
            echo.
            echo ====================================================
            echo Build successful!
            echo Run: bigbrother_viewer_new.exe
            echo ====================================================
            goto :end
        ) else (
            echo Build failed!
        )
    ) else (
        echo Visual Studio environment not properly initialized.
        echo Please run from Developer Command Prompt for VS
    )
) else (
    echo Visual Studio not found!
)

:end
pause
