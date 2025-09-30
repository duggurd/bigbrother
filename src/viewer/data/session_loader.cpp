#include "session_loader.h"
#include <windows.h>
#include <shlobj.h>
#include <fstream>
#include "json.hpp"

using json = nlohmann::json;

namespace bigbrother {
namespace viewer {

SessionLoader::SessionLoader() {
}

SessionLoader::~SessionLoader() {
}

std::vector<Session> SessionLoader::LoadFromFile(const std::string& filePath) {
    std::vector<Session> sessions;
    
    std::ifstream inFile(filePath);
    if (!inFile.is_open()) {
        return sessions; // Return empty vector
    }
    
    try {
        json data;
        inFile >> data;
        inFile.close();
        
        if (data.contains("sessions") && data["sessions"].is_array()) {
            for (const auto& sessionJson : data["sessions"]) {
                sessions.push_back(ParseSession(sessionJson));
            }
        }
    } catch (const json::exception& e) {
        // Error parsing JSON, return what we have
    }
    
    return sessions;
}

std::string SessionLoader::GetDefaultDataPath() const {
    char path[MAX_PATH];
    if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_APPDATA, NULL, 0, path))) {
        return std::string(path) + "\\BigBrother\\focus_log.json";
    }
    return "focus_log.json";
}

Session SessionLoader::ParseSession(const json& sessionJson) {
    Session session;
    session.start_timestamp = sessionJson.value("start_timestamp", 0LL);
    session.end_timestamp = sessionJson.value("end_timestamp", 0LL);
    
    if (sessionJson.contains("window_focus") && sessionJson["window_focus"].is_array()) {
        for (const auto& focusJson : sessionJson["window_focus"]) {
            session.window_focus.push_back(ParseFocusEvent(focusJson));
        }
    }
    
    return session;
}

WindowFocusEvent SessionLoader::ParseFocusEvent(const json& eventJson) {
    WindowFocusEvent event;
    event.focus_timestamp = eventJson.value("focus_timestamp", 0LL);
    event.process_name = eventJson.value("process_name", "");
    event.process_path = eventJson.value("process_path", "");
    
    if (eventJson.contains("title_changes") && eventJson["title_changes"].is_array()) {
        for (const auto& titleJson : eventJson["title_changes"]) {
            TitleChange tc;
            tc.timestamp = titleJson.value("title_timestamp", 0LL);
            tc.title = titleJson.value("window_title", "");
            event.title_changes.push_back(tc);
        }
    }
    
    // Backward compatibility: if old data has window_title in focus event,
    // move it to the first title change
    if (eventJson.contains("window_title") && !eventJson["window_title"].is_null()) {
        std::string old_title = eventJson.value("window_title", "");
        if (!old_title.empty() && event.title_changes.empty()) {
            TitleChange tc;
            tc.timestamp = event.focus_timestamp;
            tc.title = old_title;
            event.title_changes.insert(event.title_changes.begin(), tc);
        }
    }
    
    return event;
}

} // namespace viewer
} // namespace bigbrother
