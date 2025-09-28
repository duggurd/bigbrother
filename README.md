# Window Focus Monitor

A simple C++ program that monitors Windows window focus changes and prints the title of the focused window to the console.

## Requirements
- Windows operating system
- MinGW-w64 or Visual Studio C++ compiler
- Windows API libraries (user32.dll)

## Building

### Using MinGW-w64 (g++):
```bash
g++ -o window_focus_monitor.exe window_focus_monitor.cpp -luser32 -lpsapi -lshell32
```

Or simply run the provided batch file:
```bash
build.bat
```

### Using Visual Studio:
```bash
cl /EHsc window_focus_monitor.cpp user32.lib psapi.lib shell32.lib
```

## Running
```bash
window_focus_monitor.exe
```

The program will start monitoring window focus changes. Switch between different windows to see their titles and process information printed to the console. All data is automatically saved to a JSON file in your Windows AppData folder. Press Ctrl+C to exit gracefully.

## Data Storage
The program automatically saves all session data to:
`%APPDATA%\BigBrother\focus_log.json`

### JSON Structure
```json
{
  "sessions": [
    {
      "start_timestamp": 1695910245,
      "end_timestamp": 1695912000,
      "window_focus": [
        {
          "focus_timestamp": 1695910250,
          "focus_window_title": "Visual Studio Code",
          "focus_process_name": "Code.exe",
          "focus_process_path": "C:\\Users\\username\\AppData\\Local\\Programs\\Microsoft VS Code\\Code.exe"
        },
        {
          "focus_timestamp": 1695910300,
          "focus_window_title": "Google Chrome",
          "focus_process_name": "chrome.exe", 
          "focus_process_path": "C:\\Program Files\\Google\\Chrome\\Application\\chrome.exe"
        }
      ]
    }
  ]
}
```

## Console Output
```
BigBrother Window Focus Monitor Started
Press Ctrl+C to exit...
Session started. Data will be saved to: C:\Users\username\AppData\Roaming\BigBrother\focus_log.json
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
- All data is saved in real-time to a JSON file with Unix timestamps
- Sessions are tracked from program start to Ctrl+C exit
- Graceful shutdown ensures session end timestamps are recorded
- Runs a message loop to keep the program active and process events
