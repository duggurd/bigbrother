# BigBrother - Activity Tracking & Session Monitoring

Track your computer activity with detailed window focus and title change monitoring. Perfect for consultancy work tracking, productivity analysis, and time billing.

## 🚀 Quick Start

### Prerequisites
- **Windows** 10/11
- **Python 3.7+** (for building from source)

### Setup & Run (Source Code)

1. **Clone the repository**
   ```bash
   git clone https://github.com/yourusername/bigbrother.git
   cd bigbrother
   ```

2. **Install dependencies**
   ```bash
   pip install -r requirements.txt
   ```

3. **Run the application**
   ```bash
   python src/python/main.py
   ```

### 📦 Distribution & Building

#### 1. Standalone Executable (No Install)
To create a single portable `.exe`:
1. Run `build_exe.bat`.
2. Find `BigBrother.exe` in the `dist/` folder.

#### 2. Windows Installer (Setup Wizard)
To create a professional installer that adds BigBrother to the **Start Menu** and **Desktop**:

1. Install [Inno Setup 6](https://jrsoftware.org/isdl.php).
2. Run `build_installer.bat`.
3. The installer will be created at `scripts/Output/BigBrother_Setup.exe`.

## 📁 Project Structure

```
bigbrother/
├── src/
│   ├── python/          # Main Python application source
│   │   └── main.py      # Unified Monitor & Viewer logic
├── dist/                # Built executable
├── scripts/
│   ├── installer.iss    # Inno Setup configuration
│   └── Output/          # Final Installer location
├── requirements.txt     # Python dependencies
├── build_exe.bat        # Script to build standalone EXE
├── build_installer.bat  # Script to build Setup Wizard
└── README.md            # Documentation
```

## 🔧 Features

### Unified Monitor & Viewer
- **Control Panel**: Start/Stop sessions with goal descriptions.
- **Live Monitoring**: Tracks window focus and title changes in the background.
- **Timeline View**: Visual tree of sessions -> applications -> windows.
- **Time Tracking**: Precise duration and percentage breakdown.
- **Data Persistence**: Automatically saves to `%APPDATA%\BigBrother\focus_log.json`.

## 📊 Data Format

Sessions are saved to: `%APPDATA%\BigBrother\focus_log.json`

```json
{
  "sessions": [
    {
      "start_timestamp": 1727712000,
      "end_timestamp": 1727715600,
      "comment": "Working on Project X",
      "applications": [
        {
          "process_name": "Code.exe",
          "process_path": "C:\\Program Files\\Microsoft VS Code\\Code.exe",
          "total_time_spent_ms": 3600000,
          "tabs": [
            {
              "window_title": "main.py - Visual Studio Code",
              "total_time_spent_ms": 3600000
            }
          ]
        }
      ]
    }
  ]
}
```

## 🎯 Use Cases

- **Consultancy Billing** - Track time per client/project
- **Productivity Analysis** - See where time actually goes
- **Work Documentation** - Generate activity summaries
- **Time Tracking** - Detailed work session records

## 📝 License

MIT License - See LICENSE file for details

---

**Note:** This is a Windows-only application due to its reliance on Windows-specific APIs.
