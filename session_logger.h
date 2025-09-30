#pragma once

#include <windows.h>
#include <string>
#include <psapi.h>
#include <fstream>
#include <chrono>
#include <shlobj.h>
#include "json.hpp"

using json = nlohmann::json;

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

    long long GetUnixTimestamp() {
        auto now = std::chrono::system_clock::now();
        auto duration = now.time_since_epoch();
        return std::chrono::duration_cast<std::chrono::seconds>(duration).count();
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
        std::ifstream inFile(m_dataFilePath);
        if (inFile.is_open()) {
            inFile.seekg(0, std::ios::end);
            std::streampos fileSize = inFile.tellg();
            inFile.seekg(0, std::ios::beg);
            
            if (fileSize == 0) {
                m_sessionsData = json::object();
                m_sessionsData["sessions"] = json::array();
                inFile.close();
                return;
            }
            
            try {
                inFile >> m_sessionsData;
                inFile.close();
            } catch (const json::exception& e) {
                m_sessionsData = json::object();
                m_sessionsData["sessions"] = json::array();
            }
        } else {
            m_sessionsData = json::object();
            m_sessionsData["sessions"] = json::array();
        }
    }

    void SaveSessionsToFile() {
        std::ofstream outFile(m_dataFilePath);
        if (outFile.is_open()) {
            outFile << m_sessionsData.dump(2) << std::endl;
            outFile.close();
        }
    }

    void LogFocusChange(const std::string& windowTitle, const std::string& processName, const std::string& processPath) {
        if (!m_sessionActive) return;
        
        long long timestamp = GetUnixTimestamp();
        
        if (m_hasActiveFocusEvent) {
            m_currentSession["window_focus"].push_back(m_currentFocusEvent);
        }
        
        m_currentFocusEvent = json::object();
        m_currentFocusEvent["focus_timestamp"] = timestamp;
        m_currentFocusEvent["process_name"] = processName;
        m_currentFocusEvent["process_path"] = processPath;
        m_currentFocusEvent["title_changes"] = json::array();
        
        m_hasActiveFocusEvent = true;
        
        LogTitleChange(windowTitle);
    }

    void LogTitleChange(const std::string& windowTitle) {
        if (!m_sessionActive || !m_hasActiveFocusEvent) return;
        
        long long timestamp = GetUnixTimestamp();
        
        json titleChange = json::object();
        titleChange["title_timestamp"] = timestamp;
        titleChange["window_title"] = windowTitle;
        
        m_currentFocusEvent["title_changes"].push_back(titleChange);
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

    bool StartSession() {
        if (m_sessionActive) {
            return false; // Already active
        }

        m_sessionStart = GetUnixTimestamp();
        m_sessionActive = true;
        
        m_dataFilePath = GetUserDataPath();
        LoadExistingSessions();
        
        m_currentSession = json::object();
        m_currentSession["start_timestamp"] = m_sessionStart;
        m_currentSession["end_timestamp"] = nullptr;
        m_currentSession["window_focus"] = json::array();
        
        m_hasActiveFocusEvent = false;
        
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
        
        long long sessionEnd = GetUnixTimestamp();
        m_sessionActive = false;
        
        if (m_hasActiveFocusEvent) {
            m_currentSession["window_focus"].push_back(m_currentFocusEvent);
        }
        
        m_currentSession["end_timestamp"] = sessionEnd;
        m_sessionsData["sessions"].push_back(m_currentSession);
        
        SaveSessionsToFile();
        
        if (m_hFocusHook) {
            UnhookWinEvent(m_hFocusHook);
            m_hFocusHook = NULL;
        }
        if (m_hTitleHook) {
            UnhookWinEvent(m_hTitleHook);
            m_hTitleHook = NULL;
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

// Static member initialization
SessionLogger* SessionLogger::s_instance = nullptr;
