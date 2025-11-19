@echo off
pushd "%~dp0"
echo Installing dependencies...
pip install -r requirements.txt
pip install pyinstaller Pillow

echo.
echo Converting icon...
python scripts/convert_icon.py

echo.
echo Building executable...
pyinstaller --noconfirm --onefile --windowed --name "BigBrother" --icon="src/python/bigbrother.ico" src/python/main.py

echo.
echo Build complete!
echo You can find the executable in the "dist" folder.
popd
pause
