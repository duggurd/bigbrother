# Session Logger Refactoring

## What Changed

The logging functionality has been refactored into a shared library to avoid code duplication between the CLI and GUI applications.

## New Files

### `session_logger.h`
- **Shared library** containing all session logging logic
- Class-based design (`SessionLogger`) for clean encapsulation
- Handles Windows event hooks, JSON operations, and file I/O
- Used by both CLI and GUI applications

## Updated Files

### `window_focus_monitor.cpp` (CLI Application)
- **Simplified from 373 lines to ~50 lines**
- Now uses `SessionLogger` class
- Maintains same functionality
- Cleaner, more maintainable code

### `focus_log_viewer.cpp` (GUI Application)
- Added `#include "session_logger.h"`
- Added `SessionLogger g_sessionLogger` instance
- Added UI controls in menu bar:
  - **"Start Session"** button when not recording
  - **"Recording..."** indicator (green) when active
  - **"Stop Session"** button when recording
- Auto-reloads data when session is stopped

## Usage

### CLI Application (unchanged)
```bash
window_focus_monitor.exe
```
- Starts monitoring immediately
- Press Ctrl+C to stop

### GUI Application (new feature!)
```bash
focus_log_viewer.exe
```
- Click **"Start Session"** in menu bar to begin recording
- Green **"Recording..."** indicator shows active state
- Click **"Stop Session"** to end recording
- Data automatically refreshes to show new session

## Benefits

✅ **DRY Principle** - No code duplication  
✅ **Single Source of Truth** - All logging logic in one place  
✅ **Easy Maintenance** - Fix bugs once, both apps benefit  
✅ **Consistent Behavior** - CLI and GUI use identical logging  
✅ **GUI Can Record** - No need to run separate CLI app  
✅ **Cleaner Code** - Both apps significantly simplified  

## Technical Details

### SessionLogger Class
- **Public methods:**
  - `bool StartSession()` - Begin recording
  - `void StopSession()` - End recording and save
  - `bool IsSessionActive()` - Check recording status
  - `std::string GetDataFilePath()` - Get log file path
  - `long long GetSessionStartTime()` - Get session start time

- **Private implementation:**
  - Windows event hooks (focus + title changes)
  - JSON serialization/deserialization
  - File I/O operations
  - Process information extraction

### Static Instance Pattern
- Uses static pointer for Windows callback compatibility
- Singleton-like behavior ensures one logger per app
- Callbacks route through static member function

## Building

No changes to build process required. Just rebuild both applications:

```bash
# CLI
build.bat

# GUI (from Developer Command Prompt)
build_viewer.bat
```

Both applications will automatically use the shared `session_logger.h` header.

## Future Enhancements

With this refactoring, adding new features is now easier:
- Add feature to `session_logger.h` once
- Both CLI and GUI get the feature automatically
- Examples:
  - Git integration
  - URL capture for browsers
  - Working directory tracking
  - Custom event filtering
