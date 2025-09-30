#pragma once

#include <string>
#include <vector>

namespace bigbrother {

// Represents a single title change within a focused window
struct TitleChange {
    long long timestamp;
    std::string title;
};

// Represents a window focus event with its title changes
struct WindowFocusEvent {
    long long focus_timestamp;
    std::string process_name;
    std::string process_path;
    std::vector<TitleChange> title_changes;
};

// Represents a complete session with multiple focus events
struct Session {
    long long start_timestamp;
    long long end_timestamp;
    std::vector<WindowFocusEvent> window_focus;
};

} // namespace bigbrother
