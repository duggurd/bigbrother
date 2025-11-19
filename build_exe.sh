#!/bin/bash
set -e

echo "Installing dependencies with uv..."

# Check if uv is installed
if ! command -v uv &> /dev/null; then
    echo "uv is not installed. Installing uv..."
    pip install uv
fi

echo
echo "Syncing dependencies..."
uv sync

echo
echo "Converting icon..."
uv run python scripts/convert_icon.py

echo
echo "Building executable..."
# Note: pyinstaller cross-compilation from Linux to Windows is complex and often discouraged.
# This script assumes it is running in an environment (like Git Bash, WSL, or Linux) 
# that can execute Windows binaries OR creates a Linux binary if run on Linux.
# Since the project uses Windows-specific libraries (pywin32), this will fail on native Linux.
# This script is primarily for users running Bash on Windows (e.g. Git Bash).

uv run pyinstaller --noconfirm --onefile --windowed --name "BigBrother" --icon="src/python/bigbrother.ico" src/python/main.py

echo
echo "Build complete!"
echo "You can find the executable in the \"dist\" folder."

