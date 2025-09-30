#include <windows.h>
#include <iostream>
#include <string>
#include <psapi.h>
#include <fstream>
#include <vector>
#include <chrono>
#include <shlobj.h>
#include "json.hpp"

using json = nlohmann::json;

// Global variables
HWINEVENTHOOK g_hFocusHook = NULL;
HWINEVENTHOOK g_hTitleHook = NULL;
std::string g_dataFilePath;
long long g_sessionStart = 0;
bool g_sessionActive = false;
bool g_shouldExit = false;
bool g_shutdownInProgress = false;
std::string g_lastFocusedWindowTitle = "";
HWND g_lastFocusedWindow = NULL;

// JSON data structures
json g_sessionsData;
json g_currentSession;
json g_currentFocusEvent;
bool g_hasActiveFocusEvent = false;

// Function to get window title from window handle
std::string GetWindowTitle(HWND hwnd) {
    char windowTitle[256];
    int length = GetWindowTextA(hwnd, windowTitle, sizeof(windowTitle));
    if (length > 0) {
        return std::string(windowTitle);
    }
    return "Unknown Window";
}

// Function to get process name and path from window handle
std::string GetProcessInfo(HWND hwnd) {
    DWORD processId = 0;
    GetWindowThreadProcessId(hwnd, &processId);
    
    if (processId == 0) {
        return "Unknown Process";
    }
    
    // Open the process with query information access
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processId);
    if (hProcess == NULL) {
        return "Access Denied (PID: " + std::to_string(processId) + ")";
    }
    
    // Get the process executable path
    char processPath[MAX_PATH];
    DWORD pathLength = GetModuleFileNameExA(hProcess, NULL, processPath, MAX_PATH);
    
    CloseHandle(hProcess);
    
    if (pathLength > 0) {
        // Extract just the filename from the full path
        std::string fullPath(processPath);
        size_t lastSlash = fullPath.find_last_of("\\/");
        std::string processName = (lastSlash != std::string::npos) ? 
                                 fullPath.substr(lastSlash + 1) : fullPath;
        
        return processName + " (" + fullPath + ")";
    }
    
    return "Unknown Process (PID: " + std::to_string(processId) + ")";
}

// Function to get current Unix timestamp
long long GetUnixTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto duration = now.time_since_epoch();
    return std::chrono::duration_cast<std::chrono::seconds>(duration).count();
}

// Function to get user data directory path
std::string GetUserDataPath() {
    char path[MAX_PATH];
    if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_APPDATA, NULL, 0, path))) {
        std::string userDataPath = std::string(path) + "\\BigBrother";
        CreateDirectoryA(userDataPath.c_str(), NULL); // Create directory if it doesn't exist
        return userDataPath + "\\focus_log.json";
    }
    return "focus_log.json"; // Fallback to current directory
}

// Function to load existing sessions from file
void LoadExistingSessions() {
    std::ifstream inFile(g_dataFilePath);
    if (inFile.is_open()) {
        // Check if file is empty
        inFile.seekg(0, std::ios::end);
        std::streampos fileSize = inFile.tellg();
        inFile.seekg(0, std::ios::beg);
        
        if (fileSize == 0) {
            // Empty file, initialize new structure
            g_sessionsData = json::object();
            g_sessionsData["sessions"] = json::array();
            inFile.close();
            return;
        }
        
        try {
            inFile >> g_sessionsData;
            inFile.close();
        } catch (const json::exception& e) {
            std::cerr << "Error parsing existing JSON file: " << e.what() << std::endl;
            std::cerr << "Starting with a fresh sessions file." << std::endl;
            g_sessionsData = json::object();
            g_sessionsData["sessions"] = json::array();
        }
    } else {
        // File doesn't exist, initialize new structure
        g_sessionsData = json::object();
        g_sessionsData["sessions"] = json::array();
    }
}

// Function to save sessions to file
void SaveSessionsToFile() {
    std::ofstream outFile(g_dataFilePath);
    if (outFile.is_open()) {
        outFile << g_sessionsData.dump(2) << std::endl; // Pretty print with 2-space indent
        outFile.close();
    } else {
        std::cerr << "Error: Could not open file for writing: " << g_dataFilePath << std::endl;
    }
}

// Function to start a new session
void StartSession() {
    g_sessionStart = GetUnixTimestamp();
    g_sessionActive = true;
    
    g_dataFilePath = GetUserDataPath();
    
    // Load existing sessions
    LoadExistingSessions();
    
    // Create new session object
    g_currentSession = json::object();
    g_currentSession["start_timestamp"] = g_sessionStart;
    g_currentSession["end_timestamp"] = nullptr;
    g_currentSession["window_focus"] = json::array();
    
    g_hasActiveFocusEvent = false;
    
    std::cout << "Session started. Data will be saved to: " << g_dataFilePath << std::endl;
}

// Function to log focus change
void LogFocusChange(const std::string& windowTitle, const std::string& processName, const std::string& processPath) {
    if (!g_sessionActive) return;
    
    long long timestamp = GetUnixTimestamp();
    
    // Save previous focus event if it exists
    if (g_hasActiveFocusEvent) {
        g_currentSession["window_focus"].push_back(g_currentFocusEvent);
    }
    
    // Create new focus event
    g_currentFocusEvent = json::object();
    g_currentFocusEvent["focus_timestamp"] = timestamp;
    g_currentFocusEvent["window_title"] = windowTitle;
    g_currentFocusEvent["process_name"] = processName;
    g_currentFocusEvent["process_path"] = processPath;
    g_currentFocusEvent["title_changes"] = json::array();
    
    g_hasActiveFocusEvent = true;
}

// Function to log title change
void LogTitleChange(const std::string& windowTitle) {
    if (!g_sessionActive || !g_hasActiveFocusEvent) return;
    
    long long timestamp = GetUnixTimestamp();
    
    json titleChange = json::object();
    titleChange["title_timestamp"] = timestamp;
    titleChange["window_title"] = windowTitle;
    
    g_currentFocusEvent["title_changes"].push_back(titleChange);
}

// Function to end session
void EndSession() {
    if (!g_sessionActive) return;
    
    long long sessionEnd = GetUnixTimestamp();
    g_sessionActive = false;
    
    // Save the last focus event if it exists
    if (g_hasActiveFocusEvent) {
        g_currentSession["window_focus"].push_back(g_currentFocusEvent);
    }
    
    // Set the end timestamp
    g_currentSession["end_timestamp"] = sessionEnd;
    
    // Add current session to sessions array
    g_sessionsData["sessions"].push_back(g_currentSession);
    
    // Save to file
    SaveSessionsToFile();
    
    std::cout << "Session ended and data saved to: " << g_dataFilePath << std::endl;
}

// Console control handler for Ctrl+C
BOOL WINAPI ConsoleCtrlHandler(DWORD dwCtrlType) {
    switch (dwCtrlType) {
        case CTRL_C_EVENT:
        case CTRL_CLOSE_EVENT:
        case CTRL_BREAK_EVENT:
        case CTRL_LOGOFF_EVENT:
        case CTRL_SHUTDOWN_EVENT:
            // Prevent multiple shutdown calls
            if (g_shutdownInProgress) {
                return TRUE;
            }
            g_shutdownInProgress = true;
            
            std::cout << "\nShutting down gracefully..." << std::endl;
            g_shouldExit = true;
            EndSession();
            if (g_hFocusHook) {
                UnhookWinEvent(g_hFocusHook);
                g_hFocusHook = NULL;
            }
            if (g_hTitleHook) {
                UnhookWinEvent(g_hTitleHook);
                g_hTitleHook = NULL;
            }
            
            std::cout << "Program exited successfully." << std::endl;
            
            // Force exit the process
            ExitProcess(0);
        default:
            return FALSE;
    }
}

// Callback function for window events
void CALLBACK WinEventProc(
    HWINEVENTHOOK hWinEventHook,
    DWORD event,
    HWND hwnd,
    LONG idObject,
    LONG idChild,
    DWORD dwEventThread,
    DWORD dwmsEventTime
) {
    if (hwnd == NULL) return;
    
    std::string windowTitle = GetWindowTitle(hwnd);
    std::string processInfo = GetProcessInfo(hwnd);
    
    // Extract process name and path from processInfo
    std::string processName, processPath;
    size_t parenPos = processInfo.find(" (");
    if (parenPos != std::string::npos) {
        processName = processInfo.substr(0, parenPos);
        size_t pathStart = parenPos + 2;
        size_t pathEnd = processInfo.find(")", pathStart);
        if (pathEnd != std::string::npos) {
            processPath = processInfo.substr(pathStart, pathEnd - pathStart);
        }
    } else {
        processName = processInfo;
        processPath = "Unknown";
    }
    
    // Handle focus change events
    if (event == EVENT_SYSTEM_FOREGROUND && idObject == OBJID_WINDOW) {
        g_lastFocusedWindow = hwnd;
        g_lastFocusedWindowTitle = windowTitle;
        
        // Log to JSON
        LogFocusChange(windowTitle, processName, processPath);
        
        std::cout << "Focus changed to: " << windowTitle << std::endl;
        std::cout << "  Process: " << processInfo << std::endl;
        std::cout << "  ---" << std::endl;
    }
    // Handle title change events
    else if (event == EVENT_OBJECT_NAMECHANGE && idObject == OBJID_WINDOW) {
        // Only log title changes for the currently focused window or if it's a significant change
        if (hwnd == g_lastFocusedWindow && windowTitle != g_lastFocusedWindowTitle && !windowTitle.empty()) {
            g_lastFocusedWindowTitle = windowTitle;
            
            // Log to JSON
            LogTitleChange(windowTitle);
            
            std::cout << "Title changed to: " << windowTitle << std::endl;
            std::cout << "  Process: " << processInfo << std::endl;
            std::cout << "  ---" << std::endl;
        }
    }
}

int main() {
    std::cout << "BigBrother Window Focus & Title Monitor Started" << std::endl;
    std::cout << "Press Ctrl+C to exit..." << std::endl;
    
    // Set up console control handler for graceful shutdown
    SetConsoleCtrlHandler(ConsoleCtrlHandler, TRUE);
    
    // Start session
    StartSession();
    
    // Set up the hook to monitor foreground window changes
    g_hFocusHook = SetWinEventHook(
        EVENT_SYSTEM_FOREGROUND,    // eventMin - we only want foreground changes
        EVENT_SYSTEM_FOREGROUND,    // eventMax - same as eventMin for single event
        NULL,                       // hmodWinEventProc - NULL for out-of-context hook
        WinEventProc,              // lpfnWinEventProc - our callback function
        0,                         // idProcess - 0 for all processes
        0,                         // idThread - 0 for all threads
        WINEVENT_OUTOFCONTEXT      // dwFlags - out-of-context hook
    );
    
    // Set up the hook to monitor window title changes
    g_hTitleHook = SetWinEventHook(
        EVENT_OBJECT_NAMECHANGE,    // eventMin - window title changes
        EVENT_OBJECT_NAMECHANGE,    // eventMax - same as eventMin for single event
        NULL,                       // hmodWinEventProc - NULL for out-of-context hook
        WinEventProc,              // lpfnWinEventProc - our callback function
        0,                         // idProcess - 0 for all processes
        0,                         // idThread - 0 for all threads
        WINEVENT_OUTOFCONTEXT      // dwFlags - out-of-context hook
    );
    
    if (g_hFocusHook == NULL || g_hTitleHook == NULL) {
        std::cerr << "Failed to set up window event hooks!" << std::endl;
        return 1;
    }
    
    std::cout << "Hooks installed successfully. Monitoring window focus and title changes..." << std::endl;
    
    // Message loop to keep the program running
    MSG msg;
    while (!g_shouldExit && GetMessage(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    
    // Clean up (fallback cleanup if not already done by Ctrl+C handler)
    if (!g_shutdownInProgress) {
        if (g_sessionActive) {
            EndSession();
        }
        if (g_hFocusHook) {
            UnhookWinEvent(g_hFocusHook);
        }
        if (g_hTitleHook) {
            UnhookWinEvent(g_hTitleHook);
        }
        std::cout << "Program exited successfully." << std::endl;
    }
    
    return 0;
}