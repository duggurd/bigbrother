#pragma once

#include <string>
#include <vector>

namespace bigbrother {

// Represents a unique tab/window within an application
struct TabInfo {
    std::string window_title;
    long long total_time_spent_ms;  // Total time spent on this specific tab in milliseconds
};

// Represents an aggregated application focus with all its tabs
struct ApplicationFocusEvent {
    std::string process_name;
    std::string process_path;
    long long first_focus_time;      // First time this app was focused in the session
    long long last_focus_time;       // Last time this app was focused in the session
    long long total_time_spent_ms;   // Total time spent in this application in milliseconds
    std::vector<TabInfo> tabs;       // All unique tabs/windows viewed in this application
};

// Represents a complete session with aggregated application data
struct Session {
    long long start_timestamp;
    long long end_timestamp;
    std::string comment;  // User's description of what they're working on
    std::vector<ApplicationFocusEvent> applications;
};

} // namespace bigbrother
