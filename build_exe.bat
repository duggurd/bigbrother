@echo off
pushd "%~dp0"
echo Installing dependencies with uv...

REM Check if uv is installed
where uv >nul 2>nul
if %ERRORLEVEL% neq 0 (
    echo uv is not installed. Installing uv...
    pip install uv
)

echo.
echo Syncing dependencies...
uv sync

echo.
echo Converting icon...
uv run python scripts/convert_icon.py

echo.
echo Building executable...
uv run pyinstaller --noconfirm --onefile --windowed --name "BigBrother" --icon="src/python/bigbrother.ico" src/python/main.py

echo.
echo Build complete!
echo You can find the executable in the "dist" folder.
popd
pause
