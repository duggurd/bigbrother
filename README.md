# Window Focus Monitor

A simple C++ program that monitors Windows window focus changes and prints the title of the focused window to the console.

## Requirements
- Windows operating system
- MinGW-w64 or Visual Studio C++ compiler
- Windows API libraries (user32.dll)

## Building

### Using MinGW-w64 (g++):
```bash
g++ -o window_focus_monitor.exe window_focus_monitor.cpp -luser32 -lpsapi
```

Or simply run the provided batch file:
```bash
build.bat
```

### Using Visual Studio:
```bash
cl /EHsc window_focus_monitor.cpp user32.lib psapi.lib
```

## Running
```bash
window_focus_monitor.exe
```

The program will start monitoring window focus changes. Switch between different windows to see their titles and process information printed to the console. Press Ctrl+C to exit.

## Sample Output
```
Window Focus Monitor Started
Press Ctrl+C to exit...
Hook installed successfully. Monitoring window focus changes...
Focus changed to: Visual Studio Code
  Process: Code.exe (C:\Users\username\AppData\Local\Programs\Microsoft VS Code\Code.exe)
  ---
Focus changed to: Google Chrome
  Process: chrome.exe (C:\Program Files\Google\Chrome\Application\chrome.exe)
  ---
```

## How it works
- Uses Windows API `SetWinEventHook()` to register a callback for `EVENT_SYSTEM_FOREGROUND` events
- When focus changes, the callback retrieves:
  - Window title using `GetWindowText()`
  - Process ID using `GetWindowThreadProcessId()`
  - Process name and path using `OpenProcess()` and `GetModuleFileNameEx()`
- Runs a message loop to keep the program active and process events
