# BigBrother Window Focus & Title Monitor

A C++ program that monitors Windows window focus changes and title changes, tracking tab switches in browsers, terminal apps, and other applications that update their window titles. Includes a GUI viewer built with Dear ImGui to visualize your session data.

## Components
1. **Window Focus Monitor** (`window_focus_monitor.cpp`) - Background monitor that tracks window focus and title changes
2. **Session Viewer** (`focus_log_viewer.cpp`) - GUI application to visualize and analyze recorded sessions

## Requirements
- Windows operating system
- MinGW-w64 or Visual Studio C++ compiler
- Windows API libraries (user32.dll)
- Dear ImGui (for viewer only) - see setup instructions below

## Building

### Window Focus Monitor

#### Using the build script (recommended):
```bash
build.bat
```

#### Using MinGW-w64 (g++):
```bash
g++ -o window_focus_monitor.exe window_focus_monitor.cpp -luser32 -lpsapi -lshell32
```

#### Using Visual Studio:
```bash
cl /EHsc window_focus_monitor.cpp user32.lib psapi.lib shell32.lib
```

### Session Viewer (GUI)

First, set up Dear ImGui:
```bash
setup_imgui.bat
```

This will either automatically download ImGui (if you have git) or provide instructions to download it manually.

Then build the viewer:
```bash
build_viewer.bat
```

## Running

### Window Focus Monitor
```bash
window_focus_monitor.exe
```

The program will start monitoring window focus and title changes. Switch between different windows or tabs (in browsers, terminal apps, etc.) to see changes printed to the console. All data is automatically saved to a JSON file in your Windows AppData folder. Press Ctrl+C to exit gracefully.

### Session Viewer
```bash
focus_log_viewer.exe
```

The GUI viewer will open and automatically load your session data from `%APPDATA%\BigBrother\focus_log.json`. You can:
- Browse through all recorded sessions
- View detailed window focus events for each session
- See title changes within each focused window
- View statistics like session duration and most-used applications
- Reload the data file at any time with the "Reload" button

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
          "process_name": "Code.exe",
          "process_path": "C:\\Users\\username\\AppData\\Local\\Programs\\Microsoft VS Code\\Code.exe",
          "title_changes": [
            {
              "title_timestamp": 1695910250,
              "window_title": "Visual Studio Code"
            },
            {
              "title_timestamp": 1695910275,
              "window_title": "main.cpp - Visual Studio Code"
            },
            {
              "title_timestamp": 1695910285,
              "window_title": "README.md - Visual Studio Code"
            }
          ]
        },
        {
          "focus_timestamp": 1695910300,
          "process_name": "chrome.exe",
          "process_path": "C:\\Program Files\\Google\\Chrome\\Application\\chrome.exe",
          "title_changes": [
            {
              "title_timestamp": 1695910300,
              "window_title": "Google Chrome"
            },
            {
              "title_timestamp": 1695910310,
              "window_title": "GitHub - Google Chrome"
            },
            {
              "title_timestamp": 1695910320,
              "window_title": "Stack Overflow - Google Chrome"
            }
          ]
        }
      ]
    }
  ]
}
```

**Note:** The initial window title is now recorded as the first entry in `title_changes` array (with the same timestamp as `focus_timestamp`), making the data structure more consistent.

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
- Review your work sessions with the visual GUI viewer

## GUI Viewer Features

The ImGui-based viewer provides:
- **Session browsing** - View all recorded sessions with timestamps and durations
- **Detailed event timeline** - See every window focus change with timestamps
- **Title change tracking** - View all title changes within each focused window
- **Duration calculations** - Automatic calculation of time spent in each window
- **Statistics** - Session summaries showing most-used applications
- **Live reload** - Refresh data without restarting the viewer
- **Clean, responsive UI** - Built with Dear ImGui for a smooth experience

## Project Structure
```
bigbrother/
├── window_focus_monitor.cpp  # Main monitoring program
├── focus_log_viewer.cpp       # GUI viewer application
├── json.hpp                   # JSON library (nlohmann/json)
├── build.bat                  # Build script for monitor
├── build_viewer.bat           # Build script for viewer
├── setup_imgui.bat            # ImGui setup helper
├── imgui/                     # Dear ImGui library (after setup)
│   ├── imgui.cpp
│   ├── imgui.h
│   └── backends/
│       ├── imgui_impl_win32.cpp
│       ├── imgui_impl_win32.h
│       ├── imgui_impl_dx11.cpp
│       └── imgui_impl_dx11.h
└── README.md
```

Data is stored in: `%APPDATA%\BigBrother\focus_log.json`
