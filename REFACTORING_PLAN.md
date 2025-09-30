# BigBrother Refactoring Plan

## Current Issues
- ❌ All code in root directory
- ❌ Large monolithic files (viewer is 934 lines)
- ❌ Mixed concerns (UI + data + logic all together)
- ❌ Hard to find and modify specific functionality
- ❌ Difficult to add new features
- ❌ No clear module boundaries

## Goals
✅ Clear separation of concerns  
✅ Modular, reusable components  
✅ Easy to navigate and understand  
✅ Simple to add new features  
✅ Maintainable codebase  
✅ Professional structure  

## New Directory Structure

```
bigbrother/
├── src/
│   ├── common/              # Shared code
│   │   ├── session_logger.h/cpp       # Logging functionality
│   │   ├── session_data.h             # Data structures
│   │   ├── json_utils.h/cpp           # JSON helpers
│   │   └── time_utils.h/cpp           # Time formatting
│   │
│   ├── monitor/             # CLI application
│   │   └── main.cpp                   # Simple entry point
│   │
│   └── viewer/              # GUI application
│       ├── main.cpp                   # Entry point
│       ├── ui/
│       │   ├── main_window.h/cpp      # Main window UI
│       │   ├── settings_window.h/cpp  # Settings UI
│       │   └── timeline_view.h/cpp    # Timeline rendering
│       ├── data/
│       │   ├── session_loader.h/cpp   # Load/parse sessions
│       │   └── filter_manager.h/cpp   # Filter logic
│       └── graphics/
│           └── icon_manager.h/cpp     # Icon extraction/caching
│
├── include/                 # Public headers (if needed)
│
├── third_party/            # External dependencies
│   ├── imgui/              # Dear ImGui (gitignored)
│   └── json.hpp            # JSON library
│
├── build/                  # Build output (gitignored)
│
├── docs/                   # Documentation
│   ├── FUTURE_ENHANCEMENTS.md
│   ├── VIEWER_QUICKSTART.md
│   └── REFACTORING_NOTES.md
│
├── scripts/                # Build scripts
│   ├── build_monitor.bat
│   ├── build_viewer.bat
│   └── setup_imgui.bat
│
├── .gitignore
├── README.md
└── CMakeLists.txt          # Optional: Modern build system
```

## Module Breakdown

### Common (Shared Code)

#### `session_data.h`
```cpp
// Data structures only
struct TitleChange { ... };
struct WindowFocusEvent { ... };
struct Session { ... };
```

#### `session_logger.h/cpp`
```cpp
// Window event monitoring and logging
class SessionLogger {
    // Start/stop session
    // Hook management
    // JSON serialization
};
```

#### `json_utils.h/cpp`
```cpp
// JSON parsing helpers
Session ParseSessionJson(const json& j);
json SessionToJson(const Session& s);
```

#### `time_utils.h/cpp`
```cpp
// Time formatting functions
string FormatTimestamp(long long ts);
string FormatDuration(long long seconds);
string FormatDate(long long ts);
```

### Monitor (CLI)

#### `monitor/main.cpp` (~50 lines)
```cpp
// Simple entry point
// Create SessionLogger
// Handle Ctrl+C
// Run message loop
```

### Viewer (GUI)

#### `viewer/main.cpp` (~100 lines)
```cpp
// Application setup
// DirectX initialization
// ImGui setup
// Main loop
```

#### `ui/main_window.h/cpp`
```cpp
class MainWindow {
    void Render();
    void RenderMenuBar();
    void ShowTimeline();
};
```

#### `ui/settings_window.h/cpp`
```cpp
class SettingsWindow {
    void Render();
    void AddProgramFilter(string name);
    void DeleteFilter(int index);
};
```

#### `ui/timeline_view.h/cpp`
```cpp
class TimelineView {
    void RenderSessions(vector<Session>);
    void RenderSessionHeader(Session);
    void RenderFocusEvent(FocusEvent);
};
```

#### `data/session_loader.h/cpp`
```cpp
class SessionLoader {
    vector<Session> LoadFromFile(string path);
    void SaveToFile(string path, vector<Session>);
};
```

#### `data/filter_manager.h/cpp`
```cpp
class FilterManager {
    void AddFilter(string program, bool enabled);
    void RemoveFilter(string program);
    bool IsFiltered(string program);
    void SaveSettings();
    void LoadSettings();
};
```

#### `graphics/icon_manager.h/cpp`
```cpp
class IconManager {
    ID3D11ShaderResourceView* GetIcon(string path);
    void ClearCache();
private:
    map<string, ID3D11ShaderResourceView*> cache;
};
```

## Migration Steps

### Phase 1: Create Structure (No Code Changes)
1. Create new directory structure
2. Move existing files to appropriate locations
3. Update .gitignore
4. Move docs to docs/
5. Move build scripts to scripts/

### Phase 2: Extract Common Code
1. Create session_data.h with all data structures
2. Keep session_logger.h as-is (already good)
3. Extract time_utils from viewer
4. Extract json_utils for JSON parsing

### Phase 3: Refactor Monitor
1. Simplify main.cpp using extracted modules
2. Update build script

### Phase 4: Refactor Viewer (Incremental)
1. Extract IconManager class
2. Extract FilterManager class  
3. Extract SessionLoader class
4. Extract MainWindow class
5. Extract SettingsWindow class
6. Extract TimelineView class
7. Update main.cpp to wire everything together
8. Update build script

### Phase 5: Polish
1. Update all documentation
2. Test both applications
3. Clean up old files
4. Update README with new structure

## Benefits After Refactoring

### For Development
- ✅ Find code quickly (know exactly where to look)
- ✅ Modify specific features without touching everything
- ✅ Add new features easily (clear extension points)
- ✅ Test modules independently
- ✅ Reuse code between projects

### For Understanding
- ✅ Small, focused files (~100-200 lines each)
- ✅ Clear responsibilities (one file = one job)
- ✅ Obvious dependencies
- ✅ Self-documenting structure

### For Maintenance
- ✅ Easy to find bugs
- ✅ Safe to refactor (isolated changes)
- ✅ Simple to add developers
- ✅ Professional codebase

## File Size Targets

Current:
- focus_log_viewer.cpp: **934 lines** ❌

After refactoring:
- viewer/main.cpp: ~100 lines ✅
- ui/main_window.cpp: ~150 lines ✅
- ui/settings_window.cpp: ~100 lines ✅
- ui/timeline_view.cpp: ~200 lines ✅
- data/filter_manager.cpp: ~80 lines ✅
- data/session_loader.cpp: ~100 lines ✅
- graphics/icon_manager.cpp: ~100 lines ✅

**Total: ~830 lines across 7 focused files** (vs 934 in one file)

## Questions Before We Start

1. **Build System**: Keep batch files or use CMake?
2. **Breaking Changes**: OK to reorganize everything at once?
3. **Testing**: Should we keep old files until refactoring is complete?
4. **Priority**: Start with viewer (most complex) or monitor (simpler)?

## Recommended Approach

**Option A: Big Bang** (All at once)
- Create new structure
- Migrate all code in one session
- Delete old files
- Fast but risky

**Option B: Incremental** (Step by step)
- Keep old structure working
- Build new structure alongside
- Migrate piece by piece
- Delete old when new works
- Slower but safer

I recommend **Option B** for safety. What do you think?
