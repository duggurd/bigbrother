# BigBrother Window Focus & Title Monitor

A C++ program that monitors Windows window focus changes and title changes, tracking tab switches in browsers, terminal apps, and other applications that update their window titles.

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

The program will start monitoring window focus and title changes. Switch between different windows or tabs (in browsers, terminal apps, etc.) to see changes printed to the console. All data is automatically saved to a JSON file in your Windows AppData folder. Press Ctrl+C to exit gracefully.

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
      "window_events": [
        {
          "event_type": "focus_change",
          "timestamp": 1695910250,
          "window_title": "Visual Studio Code",
          "process_name": "Code.exe",
          "process_path": "C:\\Users\\username\\AppData\\Local\\Programs\\Microsoft VS Code\\Code.exe"
        },
        {
          "event_type": "title_change",
          "timestamp": 1695910275,
          "window_title": "main.cpp - Visual Studio Code",
          "process_name": "Code.exe",
          "process_path": "C:\\Users\\username\\AppData\\Local\\Programs\\Microsoft VS Code\\Code.exe"
        },
        {
          "event_type": "focus_change",
          "timestamp": 1695910300,
          "window_title": "Google Chrome",
          "process_name": "chrome.exe", 
          "process_path": "C:\\Program Files\\Google\\Chrome\\Application\\chrome.exe"
        },
        {
          "event_type": "title_change",
          "timestamp": 1695910310,
          "window_title": "GitHub - Google Chrome",
          "process_name": "chrome.exe",
          "process_path": "C:\\Program Files\\Google\\Chrome\\Application\\chrome.exe"
        }
      ]
    }
  ]
}
```

## Console Output
```
BigBrother Window Focus & Title Monitor Started
Press Ctrl+C to exit...
Session started. Data will be saved to: C:\Users\username\AppData\Roaming\BigBrother\focus_log.json
Hooks installed successfully. Monitoring window focus and title changes...
Focus changed to: Visual Studio Code
  Process: Code.exe (C:\Users\username\AppData\Local\Programs\Microsoft VS Code\Code.exe)
  ---
Title changed to: main.cpp - Visual Studio Code
  Process: Code.exe (C:\Users\username\AppData\Local\Programs\Microsoft VS Code\Code.exe)
  ---
Focus changed to: Google Chrome
  Process: chrome.exe (C:\Program Files\Google\Chrome\Application\chrome.exe)
  ---
Title changed to: GitHub - Google Chrome
  Process: chrome.exe (C:\Program Files\Google\Chrome\Application\chrome.exe)
  ---
```

## How it works
- Uses Windows API `SetWinEventHook()` to register callbacks for:
  - `EVENT_SYSTEM_FOREGROUND` events (window focus changes)
  - `EVENT_OBJECT_NAMECHANGE` events (window title changes)
- When events occur, the callback retrieves:
  - Window title using `GetWindowText()`
  - Process ID using `GetWindowThreadProcessId()`
  - Process name and path using `OpenProcess()` and `GetModuleFileNameEx()`
- Only tracks title changes for the currently focused window to avoid spam
- All data is saved in real-time to a JSON file with Unix timestamps
- Sessions are tracked from program start to Ctrl+C exit
- Graceful shutdown ensures session end timestamps are recorded
- Runs a message loop to keep the program active and process events

## Use Cases
- Track productivity and time spent in different applications
- Monitor tab switches in web browsers (Chrome, Firefox, Edge)
- Track terminal tab/session changes (Windows Terminal, ConEmu, etc.)
- Monitor document switches in text editors (VS Code, Notepad++, etc.)
- Analyze workflow patterns and application usage
