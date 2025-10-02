#pragma once

#include <windows.h>
#include <string>
#include <psapi.h>
#include <fstream>
#include <chrono>
#include <shlobj.h>
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
    
    json m_sessionsData;
    json m_currentSession;
    json m_currentFocusEvent;
    bool m_hasActiveFocusEvent = false;
    
    // Incremental write support
    int m_eventsSinceLastFlush = 0;
    std::chrono::steady_clock::time_point m_lastFlushTime;
    static const int FLUSH_EVENT_THRESHOLD = 10;  // Flush every 10 events
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

    void LoadExistingSessions() {
        // Don't load all sessions into memory - just check if file exists
        // We'll append our session when saving
        std::ifstream inFile(m_dataFilePath);
        if (inFile.is_open()) {
            inFile.seekg(0, std::ios::end);
            std::streampos fileSize = inFile.tellg();
            inFile.close();
            
            if (fileSize == 0) {
                // File is empty, initialize structure
                m_sessionsData = json::object();
                m_sessionsData["sessions"] = json::array();
            } else {
                // File exists and has content, we'll load it only when needed for flushing
                m_sessionsData = json::object();
                m_sessionsData["sessions"] = json::array();
            }
        } else {
            // File doesn't exist, initialize structure
            m_sessionsData = json::object();
            m_sessionsData["sessions"] = json::array();
        }
    }

    void SaveSessionsToFile() {
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
        
        // Append our current session
        allSessions["sessions"].push_back(m_currentSession);
        
        // Write to file
        std::ofstream outFile(m_dataFilePath);
        if (outFile.is_open()) {
            outFile << allSessions.dump(2) << std::endl;
            outFile.close();
        }
    }
    
    void FlushCurrentSession() {
        if (!m_sessionActive) return;
        
        // Finalize current focus event if active
        json tempFocusEvent = m_currentFocusEvent;
        bool hadActiveFocusEvent = m_hasActiveFocusEvent;
        
        if (m_hasActiveFocusEvent) {
            m_currentSession["window_focus"].push_back(m_currentFocusEvent);
            m_hasActiveFocusEvent = false;
        }
        
        // Update end timestamp to current time
        long long currentTime = bigbrother::GetUnixTimestamp();
        m_currentSession["end_timestamp"] = currentTime;
        
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
                allSessions["sessions"][i] = m_currentSession;
                sessionExists = true;
                break;
            }
        }
        
        if (!sessionExists) {
            // Add new session
            allSessions["sessions"].push_back(m_currentSession);
        }
        
        // Write to file
        std::ofstream outFile(m_dataFilePath);
        if (outFile.is_open()) {
            outFile << allSessions.dump(2) << std::endl;
            outFile.close();
        }
        
        // Restore the focus event so we can continue adding to it
        if (hadActiveFocusEvent) {
            m_currentFocusEvent = tempFocusEvent;
            m_hasActiveFocusEvent = true;
        }
        
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
        
        long long timestamp = bigbrother::GetUnixTimestamp();
        
        if (m_hasActiveFocusEvent) {
            m_currentSession["window_focus"].push_back(m_currentFocusEvent);
        }
        
        m_currentFocusEvent = json::object();
        m_currentFocusEvent["focus_timestamp"] = timestamp;
        m_currentFocusEvent["process_name"] = processName;
        m_currentFocusEvent["process_path"] = processPath;
        m_currentFocusEvent["title_changes"] = json::array();
        
        m_hasActiveFocusEvent = true;
        m_eventsSinceLastFlush++;
        
        LogTitleChange(windowTitle);
        
        // Check if we should flush to disk
        if (ShouldFlush()) {
            FlushCurrentSession();
        }
    }

    void LogTitleChange(const std::string& windowTitle) {
        if (!m_sessionActive || !m_hasActiveFocusEvent) return;
        
        long long timestamp = bigbrother::GetUnixTimestamp();
        
        json titleChange = json::object();
        titleChange["title_timestamp"] = timestamp;
        titleChange["window_title"] = windowTitle;
        
        m_currentFocusEvent["title_changes"].push_back(titleChange);
        m_eventsSinceLastFlush++;
        
        // Check if we should flush to disk
        if (ShouldFlush()) {
            FlushCurrentSession();
        }
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
            s_instance->LogFocusChange(windowTitle, processName, processPath);
        }
        else if (event == EVENT_OBJECT_NAMECHANGE && idObject == OBJID_WINDOW) {
            if (hwnd == s_instance->m_lastFocusedWindow && 
                windowTitle != s_instance->m_lastFocusedWindowTitle && 
                !windowTitle.empty()) {
                s_instance->m_lastFocusedWindowTitle = windowTitle;
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
        LoadExistingSessions();
        
        m_currentSession = json::object();
        m_currentSession["start_timestamp"] = m_sessionStart;
        m_currentSession["end_timestamp"] = nullptr;
        m_currentSession["comment"] = comment;
        m_currentSession["window_focus"] = json::array();
        
        m_hasActiveFocusEvent = false;
        
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
        
        // Now safe to finalize session data
        long long sessionEnd = bigbrother::GetUnixTimestamp();
        m_sessionActive = false;
        
        if (m_hasActiveFocusEvent) {
            m_currentSession["window_focus"].push_back(m_currentFocusEvent);
            m_hasActiveFocusEvent = false;
        }
        
        m_currentSession["end_timestamp"] = sessionEnd;
        
        // Do a final flush - this will update the session if it already exists
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
                allSessions["sessions"][i] = m_currentSession;
                sessionExists = true;
                break;
            }
        }
        
        if (!sessionExists) {
            // Add new session
            allSessions["sessions"].push_back(m_currentSession);
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
