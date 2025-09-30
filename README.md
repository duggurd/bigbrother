# BigBrother - Activity Tracking & Session Monitoring

Track your computer activity with detailed window focus and title change monitoring. Perfect for consultancy work tracking, productivity analysis, and time billing.

## 🚀 Quick Start

### Prerequisites
- **Windows** 10/11
- **CMake** 3.15+ ([Download](https://cmake.org/download/))
- **Visual Studio** 2019/2022 or **MinGW-w64**

### Setup

1. **Clone the repository**
   ```bash
   git clone https://github.com/yourusername/bigbrother.git
   cd bigbrother
   ```

2. **Run setup**
   ```bash
   scripts\setup_project.bat
   scripts\setup_imgui.bat
   ```

3. **Build**
   ```bash
   scripts\build_cmake.bat
   ```

4. **Run**
   ```bash
   build\bin\Release\bigbrother_monitor.exe     # CLI monitor
   build\bin\Release\bigbrother_viewer.exe      # GUI viewer
   ```

## 📁 Project Structure

```
bigbrother/
├── src/
│   ├── common/          # Shared code (session logger, data structures, utilities)
│   ├── monitor/         # CLI monitoring application
│   └── viewer/          # ImGui GUI viewer application
├── third_party/         # External dependencies (ImGui, JSON)
├── docs/                # Documentation
├── scripts/             # Build and setup scripts
├── build/               # CMake build output
└── CMakeLists.txt       # Main CMake configuration
```

## 🔧 Components

### 1. Monitor (CLI)
Lightweight background service that tracks:
- Window focus changes
- Window title changes
- Process information
- Timestamps

**Usage:**
```bash
bigbrother_monitor.exe
```
Press Ctrl+C to stop and save session.

### 2. Viewer (GUI)
Beautiful ImGui interface for:
- Viewing session timeline
- Filtering applications
- Analyzing time spent
- Recording new sessions

**Features:**
- 📊 Timeline view with icons
- 🎯 Program filters
- ⏱️ Time aggregation
- 💾 Settings persistence
- 🎨 Modern UI

## 📊 Data Format

Sessions are saved to: `%APPDATA%\BigBrother\focus_log.json`

```json
{
  "sessions": [
    {
      "start_timestamp": 1727712000,
      "end_timestamp": 1727715600,
      "window_focus": [
        {
          "focus_timestamp": 1727712100,
          "process_name": "Code.exe",
          "process_path": "C:\\Program Files\\Microsoft VS Code\\Code.exe",
          "title_changes": [
            {
              "title_timestamp": 1727712100,
              "window_title": "main.cpp - Visual Studio Code"
            }
          ]
        }
      ]
    }
  ]
}
```

## 🛠️ Building from Source

### With CMake (Recommended)
```bash
mkdir build
cd build
cmake .. -G "Visual Studio 17 2022" -A x64
cmake --build . --config Release
```

### Legacy Batch Files
```bash
build.bat              # Build monitor (old method)
build_viewer.bat       # Build viewer (old method)
```

## 📖 Documentation

- [Future Enhancements](docs/FUTURE_ENHANCEMENTS.md) - Planned features
- [Refactoring Plan](docs/REFACTORING_PLAN.md) - Code structure improvements
- [Quick Start Guide](docs/VIEWER_QUICKSTART.md) - Detailed viewer guide

## 🎯 Use Cases

- **Consultancy Billing** - Track time per client/project
- **Productivity Analysis** - See where time actually goes
- **Work Documentation** - Generate activity summaries
- **Time Tracking** - Detailed work session records
- **Pattern Recognition** - Identify workflow inefficiencies

## 🔮 Future Plans

- Git integration (branch tracking, commits)
- Browser URL capture
- LLM-powered summaries
- Multi-project tagging
- Export to CSV/Markdown
- Cloud sync
- Analytics dashboard

## 📝 License

MIT License - See LICENSE file for details

## 🤝 Contributing

Contributions welcome! Please read CONTRIBUTING.md first.

## 📧 Contact

Your Name - your.email@example.com

---

**Note:** This is a Windows-only application due to its reliance on Windows-specific APIs.