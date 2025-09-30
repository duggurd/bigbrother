#pragma once

#include <string>
#include <ctime>
#include <chrono>
#include <sstream>
#include <iomanip>

namespace bigbrother {

// Get current Unix timestamp
inline long long GetUnixTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto duration = now.time_since_epoch();
    return std::chrono::duration_cast<std::chrono::seconds>(duration).count();
}

// Format timestamp as full date and time: "2025-09-30 18:18:16"
inline std::string FormatTimestamp(long long timestamp) {
    if (timestamp == 0) return "N/A";
    
    time_t rawtime = (time_t)timestamp;
    struct tm timeinfo;
    localtime_s(&timeinfo, &rawtime);
    
    char buffer[80];
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &timeinfo);
    return std::string(buffer);
}

// Format timestamp as time only: "18:18:16"
inline std::string FormatTime(long long timestamp) {
    if (timestamp == 0) return "N/A";
    
    time_t rawtime = (time_t)timestamp;
    struct tm timeinfo;
    localtime_s(&timeinfo, &rawtime);
    
    char buffer[80];
    strftime(buffer, sizeof(buffer), "%H:%M:%S", &timeinfo);
    return std::string(buffer);
}

// Format timestamp as date only: "2025-09-30"
inline std::string FormatDate(long long timestamp) {
    if (timestamp == 0) return "N/A";
    
    time_t rawtime = (time_t)timestamp;
    struct tm timeinfo;
    localtime_s(&timeinfo, &rawtime);
    
    char buffer[80];
    strftime(buffer, sizeof(buffer), "%Y-%m-%d", &timeinfo);
    return std::string(buffer);
}

// Format timestamp with day of week: "Tuesday, September 30, 2025"
inline std::string FormatDateWithDay(long long timestamp) {
    if (timestamp == 0) return "N/A";
    
    time_t rawtime = (time_t)timestamp;
    struct tm timeinfo;
    localtime_s(&timeinfo, &rawtime);
    
    char buffer[80];
    strftime(buffer, sizeof(buffer), "%A, %B %d, %Y", &timeinfo);
    return std::string(buffer);
}

// Format duration in human-readable form: "5s", "2m 30s", "1h 15m"
inline std::string FormatDuration(long long seconds) {
    if (seconds < 60) {
        return std::to_string(seconds) + "s";
    } else if (seconds < 3600) {
        long long mins = seconds / 60;
        long long secs = seconds % 60;
        return std::to_string(mins) + "m " + std::to_string(secs) + "s";
    } else {
        long long hours = seconds / 3600;
        long long mins = (seconds % 3600) / 60;
        return std::to_string(hours) + "h " + std::to_string(mins) + "m";
    }
}

} // namespace bigbrother
