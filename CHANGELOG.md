# Changelog

## Version 2.0.0 - Refactored Architecture (2025)

### 🎉 Major Refactoring
- **Complete codebase refactoring** with modern C++ architecture
- **CMake build system** replacing batch scripts
- **Modular design** with clear separation of concerns

### Monitor Application
- Refactored from 373 lines to 60 lines
- Moved to `src/monitor/main.cpp`
- Uses shared common library

### Viewer Application  
- Refactored from 934-line monolith to 9 focused modules
- New modular structure in `src/viewer/`:
  - `graphics/icon_manager` - Icon extraction and caching
  - `data/filter_manager` - Program filter management
  - `data/session_loader` - JSON parsing and loading
  - `ui/settings_window` - Settings dialog
  - `ui/timeline_view` - Timeline rendering
  - `ui/main_window` - Application coordination
  - `main.cpp` - Entry point

### Shared Common Library
- `session_logger.h` - Logging functionality
- `session_data.h` - Data structures
- `time_utils.h` - Time formatting utilities

### Features
- ✅ All original functionality preserved
- ✅ Same data format (100% compatible)
- ✅ Improved code maintainability
- ✅ Better performance through better organization
- ✅ Settings persistence for program filters
- ✅ Icon display for applications
- ✅ Aggregated time-per-title view

### Build System
- Added CMake configuration
- Created `scripts/` directory for build tools
- Legacy batch build support maintained
- Proper dependency management

### Documentation
- Updated README with new structure
- Clean, focused documentation
- Removed redundant files

### Cleanup
- Removed old monolithic source files
- Removed deprecated build scripts
- Cleaned up build artifacts
- Updated .gitignore

---

## Version 1.0.0 - Initial Release

### Features
- Window focus tracking
- Title change monitoring
- JSON data export
- CLI monitor application
- ImGui viewer application
- Program filtering
- Settings persistence

### Data Format
- JSON-based session storage
- Support for multiple sessions
- Focus events with title changes
- Unix timestamp support
