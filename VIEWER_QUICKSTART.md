# BigBrother Session Viewer - Quick Start Guide

## What's Been Created

A complete GUI application for visualizing your BigBrother session data, built with Dear ImGui and DirectX 11.

### Files Created:
1. **focus_log_viewer.cpp** - The main viewer application
2. **build_viewer.bat** - Build script for the viewer
3. **setup_imgui.bat** - Helper script to download and setup ImGui

## Getting Started

### Step 1: Setup Dear ImGui

Run the setup script:
```bash
setup_imgui.bat
```

Choose option `Y` if you have git installed (automatic download), or `N` to download manually.

**Manual Download (if needed):**
1. Go to https://github.com/ocornut/imgui
2. Click "Code" → "Download ZIP"
3. Extract and copy these files into an `imgui` folder in your project:
   - All `.cpp` and `.h` files from the main folder
   - `backends/imgui_impl_win32.*` into `imgui/backends/`
   - `backends/imgui_impl_dx11.*` into `imgui/backends/`

### Step 2: Build the Viewer

```bash
build_viewer.bat
```

This will compile the viewer using either Visual Studio or MinGW (whichever is available).

### Step 3: Run the Viewer

```bash
focus_log_viewer.exe
```

The viewer will automatically load your session data from:
`%APPDATA%\BigBrother\focus_log.json`

## Features

### UI Layout

The viewer displays:

**Top Section - Sessions List**
- All recorded sessions with start/end times
- Session duration
- Number of window changes
- Click to select and view details

**Middle Section - Window Focus Events**
- Timeline of window focus changes for selected session
- Process name, window title, and file path
- Duration spent in each window
- Expandable tree view for details

**Title Changes**
- For windows with title changes (like browsers/editors)
- Shows timestamp and new title for each change
- Nested under the parent focus event

**Bottom Section - Statistics**
- Total session time
- Most used application
- Time spent in most used app

### Controls

- **Select Session** - Click on any session in the top list
- **Expand/Collapse** - Click the tree nodes (▼/▶) to show/hide details
- **Reload** - Click "Reload" button to refresh data from file
- **Resize** - Drag window edges to resize (standard Windows app)

## Tips

1. **Live Monitoring**: Keep the viewer open while running `window_focus_monitor.exe`, then click "Reload" to see new data
2. **Navigation**: Use mouse scroll to navigate through long lists
3. **Tree Views**: Expand events to see title changes and full details
4. **Performance**: The viewer handles large session files efficiently

## Troubleshooting

**Build Errors:**
- Make sure ImGui is properly set up in the `imgui` folder
- Verify you have either Visual Studio or MinGW installed
- Check that DirectX 11 SDK is available (comes with Windows SDK)

**Viewer Won't Start:**
- Ensure you have DirectX 11 support (Windows 7+ with updates)
- Check that the focus_log.json file exists (run the monitor first)

**No Data Showing:**
- Verify the file path shown in the menu bar is correct
- Run `window_focus_monitor.exe` first to generate data
- Click "Reload" to refresh the data

## Next Steps

1. Run the window focus monitor to collect more data
2. Use the viewer to analyze your work patterns
3. Identify productivity trends
4. See which applications consume the most time

Enjoy tracking your digital life with BigBrother! 👁️

