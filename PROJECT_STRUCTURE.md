# BigBrother Project Structure

## Directory Layout

```
bigbrother/
├── src/                          # Source code
│   ├── common/                   # Shared libraries
│   │   ├── session_logger.h      # Session logging (header-only)
│   │   ├── session_data.h        # Data structures
│   │   └── time_utils.h          # Time formatting utilities
│   │
│   ├── monitor/                  # CLI monitoring application
│   │   ├── CMakeLists.txt
│   │   └── main.cpp              # Entry point (~60 lines)
│   │
│   └── viewer/                   # GUI viewer application
│       ├── CMakeLists.txt
│       ├── main.cpp              # Entry point (~180 lines)
│       │
│       ├── ui/                   # User interface modules
│       │   ├── main_window.h/cpp        # Main window coordination
│       │   ├── settings_window.h/cpp    # Settings dialog
│       │   └── timeline_view.h/cpp      # Timeline rendering
│       │
│       ├── data/                 # Data management modules
│       │   ├── session_loader.h/cpp     # Load/parse JSON
│       │   └── filter_manager.h/cpp     # Program filters
│       │
│       └── graphics/             # Graphics utilities
│           └── icon_manager.h/cpp       # Icon extraction/caching
│
├── third_party/                  # External dependencies
│   ├── json.hpp                  # nlohmann/json (single header)
│   └── imgui/                    # Dear ImGui (gitignored)
│       ├── imgui.cpp
│       ├── imgui.h
│       └── backends/
│
├── scripts/                      # Build and setup scripts
│   ├── build_cmake.bat          # CMake build (recommended)
│   ├── build_viewer_legacy.bat  # Non-CMake build
│   ├── setup_imgui.bat          # Download ImGui
│   └── setup_project.bat        # Initialize project
│
├── build/                        # Build output (gitignored)
│   └── bin/Release/
│       ├── bigbrother_monitor.exe
│       └── bigbrother_viewer.exe
│
├── CMakeLists.txt               # Main CMake configuration
├── README.md                    # Project documentation
├── CHANGELOG.md                 # Version history
├── PROJECT_STRUCTURE.md         # This file
└── .gitignore                   # Git ignore rules
```

## Module Responsibilities

### Common Library (`src/common/`)
Header-only shared utilities used by both monitor and viewer.

- **session_logger.h** - Windows event hooks, session recording, JSON serialization
- **session_data.h** - Data structures (Session, WindowFocusEvent, TitleChange)
- **time_utils.h** - Time formatting functions (FormatTimestamp, FormatDuration, etc.)

### Monitor (`src/monitor/`)
Lightweight CLI application for background monitoring.

- **main.cpp** - Simple entry point, creates SessionLogger, handles Ctrl+C, runs message loop

### Viewer (`src/viewer/`)
ImGui-based GUI application for viewing and analyzing sessions.

#### Entry Point
- **main.cpp** - DirectX setup, ImGui initialization, main render loop

#### UI Modules (`ui/`)
- **main_window.h/cpp** - Coordinates all UI components, menu bar, session controls
- **settings_window.h/cpp** - Settings dialog with program filter management
- **timeline_view.h/cpp** - Session timeline rendering with date grouping

#### Data Modules (`data/`)
- **session_loader.h/cpp** - Load and parse JSON session files
- **filter_manager.h/cpp** - Manage program filters, save/load settings

#### Graphics Modules (`graphics/`)
- **icon_manager.h/cpp** - Extract icons from executables, convert to DirectX textures

## File Count & Lines of Code

### Monitor
| File | Lines | Purpose |
|------|-------|---------|
| `monitor/main.cpp` | ~60 | Entry point |
| **Total** | **60** | **Complete CLI app** |

### Viewer
| File | Lines | Purpose |
|------|-------|---------|
| `viewer/main.cpp` | ~180 | Entry point & DirectX |
| `ui/main_window.cpp` | ~90 | Main window |
| `ui/settings_window.cpp` | ~115 | Settings UI |
| `ui/timeline_view.cpp` | ~200 | Timeline rendering |
| `data/session_loader.cpp` | ~105 | JSON loading |
| `data/filter_manager.cpp` | ~130 | Filter logic |
| `graphics/icon_manager.cpp` | ~160 | Icon handling |
| Headers (6 files) | ~300 | Interfaces |
| **Total** | **~1,280** | **Complete GUI app** |

### Common
| File | Lines | Purpose |
|------|-------|---------|
| `common/session_logger.h` | ~290 | Session recording |
| `common/session_data.h` | ~30 | Data structures |
| `common/time_utils.h` | ~80 | Time utilities |
| **Total** | **~400** | **Shared library** |

**Project Total: ~1,740 lines** (well-organized across 16 focused files)

## Build Targets

### With CMake (Recommended)
```bash
scripts\build_cmake.bat
```

Produces:
- `build/bin/Release/bigbrother_monitor.exe`
- `build/bin/Release/bigbrother_viewer.exe`

### Without CMake (Legacy)
```bash
# From Developer Command Prompt
scripts\build_viewer_legacy.bat
```

Produces:
- `bigbrother_viewer_new.exe`

## Data Files

Application data is stored in:
- **Session logs**: `%APPDATA%\BigBrother\focus_log.json`
- **Settings**: `%APPDATA%\BigBrother\viewer_settings.json`

## Key Design Principles

### 1. Separation of Concerns
Each module has a single, clear responsibility

### 2. Dependency Injection
Components receive dependencies via constructor

### 3. Header-Only Common Library
Simple, no linking issues

### 4. Clear Namespacing
```cpp
namespace bigbrother {
    // Common functionality
    
    namespace viewer {
        // Viewer-specific code
    }
}
```

### 5. RAII Resource Management
Proper cleanup in destructors

### 6. Const Correctness
Methods that don't modify state are marked const

## Adding New Features

### New UI Window
1. Create `ui/new_window.h/cpp`
2. Add to `main_window.h` as member
3. Call `Render()` in `main_window.cpp`
4. Update `CMakeLists.txt`

### New Data Source
1. Create `data/new_loader.h/cpp`
2. Add parsing logic
3. Update `session_loader.cpp` if needed

### New Graphics Feature
1. Add to `graphics/` directory
2. Inject into UI components as needed

## Dependencies

### Monitor
- Windows API (user32, psapi, shell32)
- nlohmann/json (header-only)

### Viewer  
- All monitor dependencies, plus:
- Dear ImGui (included in third_party/)
- DirectX 11 (d3d11, dxgi, d3dcompiler)

## Version History

See [CHANGELOG.md](CHANGELOG.md) for detailed version history.

## Quick Start

1. **Setup**
   ```bash
   scripts\setup_project.bat
   scripts\setup_imgui.bat
   ```

2. **Build**
   ```bash
   scripts\build_cmake.bat
   ```

3. **Run**
   ```bash
   build\bin\Release\bigbrother_monitor.exe
   build\bin\Release\bigbrother_viewer.exe
   ```

## Contributing

When adding code:
- Keep files under 200 lines when possible
- Follow existing namespace structure
- Add headers for new classes
- Update CMakeLists.txt
- Document public interfaces

## Architecture Diagram

```
┌─────────────┐
│   Monitor   │  (CLI)
│   (60 LOC)  │
└──────┬──────┘
       │
       │ uses
       ▼
  ┌─────────────────┐
  │ Common Library  │
  │  - SessionLogger│
  │  - Data Types   │
  │  - Time Utils   │
  └────────┬────────┘
           │
           │ uses
           ▼
      ┌─────────────────────┐
      │      Viewer         │
      │   (GUI - 1280 LOC)  │
      ├─────────────────────┤
      │  UI Components      │
      │  - MainWindow       │
      │  - SettingsWindow   │
      │  - TimelineView     │
      ├─────────────────────┤
      │  Data Management    │
      │  - SessionLoader    │
      │  - FilterManager    │
      ├─────────────────────┤
      │  Graphics           │
      │  - IconManager      │
      └─────────────────────┘
```
