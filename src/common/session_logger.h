#pragma once

#include <windows.h>
#include <string>
#include <psapi.h>
#include <fstream>
#include <iostream>
#include <chrono>
#include <shlobj.h>
#include <map>
#include "json.hpp"
#include "time_utils.h"

using json = nlohmann::json;

namespace bigbrother {

class SessionLogger {
private:
    HWINEVENTHOOK m_hFocusHook = NULL;
    HWINEVENTHOOK m_hTitleHook = NULL;
    std::string m_dataFilePath;
    long long m_sessionStart = 0;
    bool m_sessionActive = false;
    std::string m_lastFocusedWindowTitle = "";
    HWND m_lastFocusedWindow = NULL;
    
    // Current tracking
    std::string m_currentProcessName;
    std::string m_currentProcessPath;
    std::string m_currentWindowTitle;
    long long m_currentFocusStartTime = 0;
    
    // Aggregated session data
    struct TabData {
        long long total_time_ms = 0;
    };
    
    struct ApplicationData {
        std::string process_name;
        std::string process_path;
        long long first_focus_time = 0;
        long long last_focus_time = 0;
        long long total_time_ms = 0;
        std::map<std::string, TabData> tabs;  // window_title -> TabData
    };
    
    std::map<std::string, ApplicationData> m_applications;  // process_name -> ApplicationData
    
    // Incremental write support
    int m_eventsSinceLastFlush = 0;
    std::chrono::steady_clock::time_point m_lastFlushTime;
    static const int FLUSH_EVENT_THRESHOLD = 1;  // Flush on every event for real-time updates
    static const int FLUSH_TIME_THRESHOLD_SECONDS = 30;  // Flush every 30 seconds

    // Static instance pointer for callbacks
    static SessionLogger* s_instance;

    // Helper functions
    std::string GetWindowTitle(HWND hwnd) {
        char windowTitle[256];
        int length = GetWindowTextA(hwnd, windowTitle, sizeof(windowTitle));
        if (length > 0) {
            return std::string(windowTitle);
        }
        return "Unknown Window";
    }

    std::string GetProcessInfo(HWND hwnd) {
        DWORD processId = 0;
        GetWindowThreadProcessId(hwnd, &processId);
        
        if (processId == 0) {
            return "Unknown Process";
        }
        
        HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processId);
        if (hProcess == NULL) {
            return "Access Denied (PID: " + std::to_string(processId) + ")";
        }
        
        char processPath[MAX_PATH];
        DWORD pathLength = GetModuleFileNameExA(hProcess, NULL, processPath, MAX_PATH);
        
        CloseHandle(hProcess);
        
        if (pathLength > 0) {
            std::string fullPath(processPath);
            size_t lastSlash = fullPath.find_last_of("\\/");
            std::string processName = (lastSlash != std::string::npos) ? 
                                     fullPath.substr(lastSlash + 1) : fullPath;
            
            return processName + " (" + fullPath + ")";
        }
        
        return "Unknown Process (PID: " + std::to_string(processId) + ")";
    }

    std::string GetUserDataPath() const {
        char path[MAX_PATH];
        if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_APPDATA, NULL, 0, path))) {
            std::string userDataPath = std::string(path) + "\\BigBrother";
            CreateDirectoryA(userDataPath.c_str(), NULL);
            return userDataPath + "\\focus_log.json";
        }
        return "focus_log.json";
    }

    void FinalizeCurrentFocus() {
        if (m_currentFocusStartTime == 0 || m_currentProcessName.empty()) {
            return;  // Nothing to finalize
        }
        
        long long currentTime = bigbrother::GetUnixTimestamp();
        long long timeSpentMs = (currentTime - m_currentFocusStartTime) * 1000;  // Convert seconds to milliseconds
        
        // Get or create application entry
        ApplicationData& appData = m_applications[m_currentProcessName];
        if (appData.process_name.empty()) {
            // New application
            appData.process_name = m_currentProcessName;
            appData.process_path = m_currentProcessPath;
            appData.first_focus_time = m_currentFocusStartTime;
        }
        
        // Update application data
        appData.last_focus_time = currentTime;
        appData.total_time_ms += timeSpentMs;
        
        // Update tab data
        TabData& tabData = appData.tabs[m_currentWindowTitle];
        tabData.total_time_ms += timeSpentMs;
        
        m_eventsSinceLastFlush++;
    }
    
    json BuildSessionJSON() {
        json sessionJson = json::object();
        sessionJson["start_timestamp"] = m_sessionStart;
        sessionJson["end_timestamp"] = bigbrother::GetUnixTimestamp();
        sessionJson["comment"] = "";  // Can be set later
        sessionJson["applications"] = json::array();
        
        for (const auto& [processName, appData] : m_applications) {
            json appJson = json::object();
            appJson["process_name"] = appData.process_name;
            appJson["process_path"] = appData.process_path;
            appJson["first_focus_time"] = appData.first_focus_time;
            appJson["last_focus_time"] = appData.last_focus_time;
            appJson["total_time_spent_ms"] = appData.total_time_ms;
            appJson["tabs"] = json::array();
            
            for (const auto& [windowTitle, tabData] : appData.tabs) {
                json tabJson = json::object();
                tabJson["window_title"] = windowTitle;
                tabJson["total_time_spent_ms"] = tabData.total_time_ms;
                appJson["tabs"].push_back(tabJson);
            }
            
            sessionJson["applications"].push_back(appJson);
        }
        
        return sessionJson;
    }
    
    void FlushCurrentSession() {
        if (!m_sessionActive) return;
        
        // Finalize current focus before flushing
        long long tempFocusStartTime = m_currentFocusStartTime;
        std::string tempProcessName = m_currentProcessName;
        std::string tempProcessPath = m_currentProcessPath;
        std::string tempWindowTitle = m_currentWindowTitle;
        
        FinalizeCurrentFocus();
        
        // Build session JSON
        json sessionJson = BuildSessionJSON();
        
        // Load existing sessions from file
        json allSessions;
        std::ifstream inFile(m_dataFilePath);
        if (inFile.is_open()) {
            inFile.seekg(0, std::ios::end);
            std::streampos fileSize = inFile.tellg();
            inFile.seekg(0, std::ios::beg);
            
            if (fileSize > 0) {
                try {
                    inFile >> allSessions;
                } catch (const json::exception& e) {
                    allSessions = json::object();
                    allSessions["sessions"] = json::array();
                }
            } else {
                allSessions = json::object();
                allSessions["sessions"] = json::array();
            }
            inFile.close();
        } else {
            allSessions = json::object();
            allSessions["sessions"] = json::array();
        }
        
        // Check if we already have a session with our start timestamp (incremental update)
        bool sessionExists = false;
        for (size_t i = 0; i < allSessions["sessions"].size(); ++i) {
            if (allSessions["sessions"][i]["start_timestamp"] == m_sessionStart) {
                // Update existing session
                allSessions["sessions"][i] = sessionJson;
                sessionExists = true;
                break;
            }
        }
        
        if (!sessionExists) {
            // Add new session
            allSessions["sessions"].push_back(sessionJson);
        }
        
        // Write to file
        std::ofstream outFile(m_dataFilePath);
        if (outFile.is_open()) {
            outFile << allSessions.dump(2) << std::endl;
            outFile.close();
        } else {
            std::cerr << "[ERROR] Could not open file for writing!" << std::endl;
        }
        
        // Restore current focus tracking to continue
        m_currentFocusStartTime = bigbrother::GetUnixTimestamp();
        m_currentProcessName = tempProcessName;
        m_currentProcessPath = tempProcessPath;
        m_currentWindowTitle = tempWindowTitle;
        
        // Reset flush counter and timer
        m_eventsSinceLastFlush = 0;
        m_lastFlushTime = std::chrono::steady_clock::now();
    }
    
    bool ShouldFlush() {
        if (m_eventsSinceLastFlush >= FLUSH_EVENT_THRESHOLD) {
            return true;
        }
        
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - m_lastFlushTime);
        if (elapsed.count() >= FLUSH_TIME_THRESHOLD_SECONDS) {
            return true;
        }
        
        return false;
    }

    void LogFocusChange(const std::string& windowTitle, const std::string& processName, const std::string& processPath) {
        if (!m_sessionActive) return;
        
        // Finalize previous focus
        FinalizeCurrentFocus();
        
        // Start new focus
        m_currentProcessName = processName;
        m_currentProcessPath = processPath;
        m_currentWindowTitle = windowTitle;
        m_currentFocusStartTime = bigbrother::GetUnixTimestamp();
        
        // Always flush on every event for real-time updates
        FlushCurrentSession();
    }

    void LogTitleChange(const std::string& windowTitle) {
        if (!m_sessionActive || m_currentProcessName.empty()) return;
        
        // Only log if title actually changed
        if (windowTitle == m_currentWindowTitle) return;
        
        // Finalize current tab time
        FinalizeCurrentFocus();
        
        // Start tracking new tab in same application
        m_currentWindowTitle = windowTitle;
        m_currentFocusStartTime = bigbrother::GetUnixTimestamp();
        
        // Always flush on every event for real-time updates
        FlushCurrentSession();
    }

    static void CALLBACK WinEventProc(
        HWINEVENTHOOK hWinEventHook,
        DWORD event,
        HWND hwnd,
        LONG idObject,
        LONG idChild,
        DWORD dwEventThread,
        DWORD dwmsEventTime
    ) {
        if (!s_instance || hwnd == NULL) return;
        
        std::string windowTitle = s_instance->GetWindowTitle(hwnd);
        std::string processInfo = s_instance->GetProcessInfo(hwnd);
        
        std::string processName, processPath;
        size_t parenPos = processInfo.find(" (");
        if (parenPos != std::string::npos) {
            processName = processInfo.substr(0, parenPos);
            size_t pathStart = parenPos + 2;
            size_t pathEnd = processInfo.rfind(")");
            if (pathEnd != std::string::npos && pathEnd > pathStart) {
                processPath = processInfo.substr(pathStart, pathEnd - pathStart);
            }
        } else {
            processName = processInfo;
            processPath = "Unknown";
        }
        
        if (event == EVENT_SYSTEM_FOREGROUND && idObject == OBJID_WINDOW) {
            s_instance->m_lastFocusedWindow = hwnd;
            s_instance->m_lastFocusedWindowTitle = windowTitle;
            
            std::cout << "Focus changed to: " << windowTitle << std::endl;
            std::cout << "  Process: " << processInfo << std::endl;
            std::cout << "  ---" << std::endl;
            
            s_instance->LogFocusChange(windowTitle, processName, processPath);
        }
        else if (event == EVENT_OBJECT_NAMECHANGE && idObject == OBJID_WINDOW) {
            if (hwnd == s_instance->m_lastFocusedWindow && 
                windowTitle != s_instance->m_lastFocusedWindowTitle && 
                !windowTitle.empty()) {
                s_instance->m_lastFocusedWindowTitle = windowTitle;
                
                std::cout << "Title changed to: " << windowTitle << std::endl;
                std::cout << "  Process: " << processInfo << std::endl;
                std::cout << "  ---" << std::endl;
                
                s_instance->LogTitleChange(windowTitle);
            }
        }
    }

public:
    SessionLogger() {
        s_instance = this;
    }

    ~SessionLogger() {
        if (m_sessionActive) {
            StopSession();
        }
        s_instance = nullptr;
    }

    bool StartSession(const std::string& comment = "") {
        if (m_sessionActive) {
            return false; // Already active
        }

        m_sessionStart = bigbrother::GetUnixTimestamp();
        m_sessionActive = true;
        
        m_dataFilePath = GetUserDataPath();
        
        // Initialize tracking variables
        m_applications.clear();
        m_currentProcessName.clear();
        m_currentProcessPath.clear();
        m_currentWindowTitle.clear();
        m_currentFocusStartTime = 0;
        
        // Initialize flush tracking
        m_eventsSinceLastFlush = 0;
        m_lastFlushTime = std::chrono::steady_clock::now();
        
        // Set up hooks
        m_hFocusHook = SetWinEventHook(
            EVENT_SYSTEM_FOREGROUND,
            EVENT_SYSTEM_FOREGROUND,
            NULL,
            WinEventProc,
            0,
            0,
            WINEVENT_OUTOFCONTEXT
        );
        
        m_hTitleHook = SetWinEventHook(
            EVENT_OBJECT_NAMECHANGE,
            EVENT_OBJECT_NAMECHANGE,
            NULL,
            WinEventProc,
            0,
            0,
            WINEVENT_OUTOFCONTEXT
        );
        
        return (m_hFocusHook != NULL && m_hTitleHook != NULL);
    }

    void StopSession() {
        if (!m_sessionActive) return;
        
        // Unhook events FIRST to prevent race conditions
        if (m_hFocusHook) {
            UnhookWinEvent(m_hFocusHook);
            m_hFocusHook = NULL;
        }
        if (m_hTitleHook) {
            UnhookWinEvent(m_hTitleHook);
            m_hTitleHook = NULL;
        }
        
        m_sessionActive = false;
        
        // Finalize current focus
        FinalizeCurrentFocus();
        
        // Build final session JSON
        json sessionJson = BuildSessionJSON();
        
        // Load existing sessions from file
        json allSessions;
        std::ifstream inFile(m_dataFilePath);
        if (inFile.is_open()) {
            inFile.seekg(0, std::ios::end);
            std::streampos fileSize = inFile.tellg();
            inFile.seekg(0, std::ios::beg);
            
            if (fileSize > 0) {
                try {
                    inFile >> allSessions;
                } catch (const json::exception& e) {
                    allSessions = json::object();
                    allSessions["sessions"] = json::array();
                }
            } else {
                allSessions = json::object();
                allSessions["sessions"] = json::array();
            }
            inFile.close();
        } else {
            allSessions = json::object();
            allSessions["sessions"] = json::array();
        }
        
        // Check if we already have a session with our start timestamp
        bool sessionExists = false;
        for (size_t i = 0; i < allSessions["sessions"].size(); ++i) {
            if (allSessions["sessions"][i]["start_timestamp"] == m_sessionStart) {
                // Update existing session
                allSessions["sessions"][i] = sessionJson;
                sessionExists = true;
                break;
            }
        }
        
        if (!sessionExists) {
            // Add new session
            allSessions["sessions"].push_back(sessionJson);
        }
        
        // Write to file
        std::ofstream outFile(m_dataFilePath);
        if (outFile.is_open()) {
            outFile << allSessions.dump(2) << std::endl;
            outFile.close();
        }
    }

    bool IsSessionActive() const {
        return m_sessionActive;
    }

    std::string GetDataFilePath() const {
        return m_dataFilePath.empty() ? GetUserDataPath() : m_dataFilePath;
    }

    long long GetSessionStartTime() const {
        return m_sessionStart;
    }
};

// Static member initialization (inline to avoid multiple definition errors)
inline SessionLogger* SessionLogger::s_instance = nullptr;

} // namespace bigbrother
