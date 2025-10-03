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
    session.comment = sessionJson.value("comment", "");
    
    if (sessionJson.contains("applications") && sessionJson["applications"].is_array()) {
        for (const auto& appJson : sessionJson["applications"]) {
            session.applications.push_back(ParseApplicationEvent(appJson));
        }
    }
    
    return session;
}

ApplicationFocusEvent SessionLoader::ParseApplicationEvent(const json& appJson) {
    ApplicationFocusEvent app;
    app.process_name = appJson.value("process_name", "");
    app.process_path = appJson.value("process_path", "");
    app.first_focus_time = appJson.value("first_focus_time", 0LL);
    app.last_focus_time = appJson.value("last_focus_time", 0LL);
    app.total_time_spent_ms = appJson.value("total_time_spent_ms", 0LL);
    
    if (appJson.contains("tabs") && appJson["tabs"].is_array()) {
        for (const auto& tabJson : appJson["tabs"]) {
            TabInfo tab;
            tab.window_title = tabJson.value("window_title", "");
            tab.total_time_spent_ms = tabJson.value("total_time_spent_ms", 0LL);
            app.tabs.push_back(tab);
        }
    }
    
    return app;
}

bool SessionLoader::SaveToFile(const std::string& filePath, const std::vector<Session>& sessions) {
    try {
        json data;
        data["sessions"] = json::array();
        
        // Convert sessions to JSON
        for (const auto& session : sessions) {
            json sessionJson;
            sessionJson["start_timestamp"] = session.start_timestamp;
            sessionJson["end_timestamp"] = session.end_timestamp;
            sessionJson["comment"] = session.comment;
            sessionJson["applications"] = json::array();
            
            // Convert application events
            for (const auto& app : session.applications) {
                json appJson;
                appJson["process_name"] = app.process_name;
                appJson["process_path"] = app.process_path;
                appJson["first_focus_time"] = app.first_focus_time;
                appJson["last_focus_time"] = app.last_focus_time;
                appJson["total_time_spent_ms"] = app.total_time_spent_ms;
                appJson["tabs"] = json::array();
                
                // Convert tabs
                for (const auto& tab : app.tabs) {
                    json tabJson;
                    tabJson["window_title"] = tab.window_title;
                    tabJson["total_time_spent_ms"] = tab.total_time_spent_ms;
                    appJson["tabs"].push_back(tabJson);
                }
                
                sessionJson["applications"].push_back(appJson);
            }
            
            data["sessions"].push_back(sessionJson);
        }
        
        // Write to file
        std::ofstream outFile(filePath);
        if (!outFile.is_open()) {
            return false;
        }
        
        outFile << data.dump(2); // Pretty print with 2-space indent
        outFile.close();
        return true;
    } catch (const json::exception& e) {
        return false;
    }
}

bool SessionLoader::DeleteSession(std::vector<Session>& sessions, int sessionIndex) {
    if (sessionIndex < 0 || sessionIndex >= sessions.size()) {
        return false;
    }
    
    sessions.erase(sessions.begin() + sessionIndex);
    return true;
}

} // namespace viewer
} // namespace bigbrother
