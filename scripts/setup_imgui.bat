@echo off
echo ====================================================
echo  Dear ImGui Setup Script for BigBrother Viewer
echo ====================================================
echo.
echo This script will help you set up Dear ImGui for the viewer.
echo.
echo Please follow these steps:
echo.
echo 1. Download Dear ImGui from: https://github.com/ocornut/imgui
echo    - Click "Code" -^> "Download ZIP"
echo    - Or clone: git clone https://github.com/ocornut/imgui.git
echo.
echo 2. Extract the ZIP file
echo.
echo 3. Copy the following files to this directory:
echo.
echo    From the main ImGui folder, create an 'imgui' folder here and copy:
echo    - imgui.cpp
echo    - imgui_demo.cpp
echo    - imgui_draw.cpp
echo    - imgui_tables.cpp
echo    - imgui_widgets.cpp
echo    - imgui.h
echo    - imconfig.h
echo    - imgui_internal.h
echo    - imstb_rectpack.h
echo    - imstb_textedit.h
echo    - imstb_truetype.h
echo.
echo    From the 'backends' subfolder, create 'imgui\backends' here and copy:
echo    - imgui_impl_win32.cpp
echo    - imgui_impl_win32.h
echo    - imgui_impl_dx11.cpp
echo    - imgui_impl_dx11.h
echo.
echo Your directory structure should look like:
echo   bigbrother\
echo     imgui\
echo       imgui.cpp
echo       imgui.h
echo       ... (other imgui files)
echo       backends\
echo         imgui_impl_win32.cpp
echo         imgui_impl_win32.h
echo         imgui_impl_dx11.cpp
echo         imgui_impl_dx11.h
echo     focus_log_viewer.cpp
echo     build_viewer.bat
echo.
echo ====================================================
echo.
echo Would you like to automatically download ImGui? (Requires git)
echo.
set /p choice="Enter Y to download with git, or N to do it manually: "

if /i "%choice%"=="Y" (
    echo.
    echo Checking for git...
    where git >nul 2>nul
    if %ERRORLEVEL% EQU 0 (
        echo Git found! Cloning Dear ImGui...
        git clone --depth 1 https://github.com/ocornut/imgui.git imgui_temp
        
        echo Creating imgui directory structure...
        mkdir imgui 2>nul
        mkdir imgui\backends 2>nul
        
        echo Copying files...
        copy imgui_temp\*.cpp imgui\ >nul
        copy imgui_temp\*.h imgui\ >nul
        copy imgui_temp\backends\imgui_impl_win32.* imgui\backends\ >nul
        copy imgui_temp\backends\imgui_impl_dx11.* imgui\backends\ >nul
        
        echo Cleaning up...
        rmdir /s /q imgui_temp
        
        echo.
        echo ====================================================
        echo  Setup complete! You can now run build_viewer.bat
        echo ====================================================
    ) else (
        echo Git not found! Please download manually.
    )
) else (
    echo.
    echo Please download and set up ImGui manually as described above.
)

echo.
pause

