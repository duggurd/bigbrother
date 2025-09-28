#include <windows.h>
#include <iostream>
#include <string>
#include <psapi.h>
#include <fstream>
#include <vector>
#include <chrono>
#include <shlobj.h>

// Global variables
HWINEVENTHOOK g_hFocusHook = NULL;
HWINEVENTHOOK g_hTitleHook = NULL;
std::string g_dataFilePath;
std::ofstream g_logFile;
long long g_sessionStart = 0;
bool g_sessionActive = false;
bool g_shouldExit = false;
bool g_shutdownInProgress = false;
std::string g_lastFocusedWindowTitle = "";
HWND g_lastFocusedWindow = NULL;

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

// Function to escape JSON strings
std::string EscapeJsonString(const std::string& str) {
    std::string escaped;
    for (char c : str) {
        switch (c) {
            case '"': escaped += "\\\""; break;
            case '\\': escaped += "\\\\"; break;
            case '\b': escaped += "\\b"; break;
            case '\f': escaped += "\\f"; break;
            case '\n': escaped += "\\n"; break;
            case '\r': escaped += "\\r"; break;
            case '\t': escaped += "\\t"; break;
            default: escaped += c; break;
        }
    }
    return escaped;
}

// Function to start a new session
void StartSession() {
    g_sessionStart = GetUnixTimestamp();
    g_sessionActive = true;
    
    g_dataFilePath = GetUserDataPath();
    
    // Read existing data
    std::ifstream inFile(g_dataFilePath);
    std::string existingData;
    if (inFile.is_open()) {
        std::string line;
        while (std::getline(inFile, line)) {
            existingData += line + "\n";
        }
        inFile.close();
    }
    
    // Open file for writing
    g_logFile.open(g_dataFilePath, std::ios::out | std::ios::trunc);
    
    if (existingData.empty()) {
        // New file - start with sessions array
        g_logFile << "{ \"sessions\": [\n";
        g_logFile << "{\"start_timestamp\": " << g_sessionStart 
                 << ", \"end_timestamp\": null, \"window_focus\": [";
    } else {
        // Existing file - find the last session and close it, then add new session
        size_t lastBracket = existingData.rfind(']');
        if (lastBracket != std::string::npos) {
            // Remove the closing brackets and add new session
            existingData = existingData.substr(0, lastBracket);
            g_logFile << existingData;
            if (existingData.find("window_events") != std::string::npos || existingData.find("window_focus") != std::string::npos) {
                g_logFile << "],\n"; // Close previous session
            }
            g_logFile << "{\"start_timestamp\": " << g_sessionStart 
                     << ", \"end_timestamp\": null, \"window_focus\": [";
        } else {
            // Corrupted file, start fresh
            g_logFile << "{ \"sessions\": [\n";
            g_logFile << "{\"start_timestamp\": " << g_sessionStart 
                     << ", \"end_timestamp\": null, \"window_focus\": [";
        }
    }
    
    g_logFile.flush();
    std::cout << "Session started. Data will be saved to: " << g_dataFilePath << std::endl;
}

// Function to log window event (focus change or title change)
void LogWindowEvent(const std::string& eventType, const std::string& windowTitle, const std::string& processName, const std::string& processPath) {
    if (!g_sessionActive) return;
    
    long long timestamp = GetUnixTimestamp();
    
    // Check if this is the first event in the session
    static bool firstEvent = true;
    if (!firstEvent) {
        g_logFile << ",\n";
    }
    firstEvent = false;
    
    g_logFile << "{\"event_type\": \"" << eventType << "\""
             << ", \"timestamp\": " << timestamp
             << ", \"window_title\": \"" << EscapeJsonString(windowTitle) << "\""
             << ", \"process_name\": \"" << EscapeJsonString(processName) << "\""
             << ", \"process_path\": \"" << EscapeJsonString(processPath) << "\"}";
    
    g_logFile.flush();
}

// Function to end session
void EndSession() {
    if (!g_sessionActive) return;
    
    long long sessionEnd = GetUnixTimestamp();
    g_sessionActive = false;
    
    if (g_logFile.is_open()) {
        // Close the current session
        g_logFile << "]}";
        g_logFile.close();
        
        // Read the file back and update the end timestamp
        std::ifstream inFile(g_dataFilePath);
        std::string content;
        if (inFile.is_open()) {
            std::string line;
            while (std::getline(inFile, line)) {
                content += line + "\n";
            }
            inFile.close();
        }
        
        // Replace "end_timestamp": null with actual timestamp
        size_t pos = content.find("\"end_timestamp\": null");
        if (pos != std::string::npos) {
            content.replace(pos, 22, "\"end_timestamp\": " + std::to_string(sessionEnd));
        }
        
        // Add closing brackets if needed
        if (content.back() != '\n') content += "\n";
        content += "]}\n";
        
        // Write back to file
        std::ofstream outFile(g_dataFilePath);
        if (outFile.is_open()) {
            outFile << content;
            outFile.close();
        }
    }
    
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
        
        // Log to JSON file
        LogWindowEvent("focus_change", windowTitle, processName, processPath);
        
        std::cout << "Focus changed to: " << windowTitle << std::endl;
        std::cout << "  Process: " << processInfo << std::endl;
        std::cout << "  ---" << std::endl;
    }
    // Handle title change events
    else if (event == EVENT_OBJECT_NAMECHANGE && idObject == OBJID_WINDOW) {
        // Only log title changes for the currently focused window or if it's a significant change
        if (hwnd == g_lastFocusedWindow && windowTitle != g_lastFocusedWindowTitle && !windowTitle.empty()) {
            g_lastFocusedWindowTitle = windowTitle;
            
            // Log to JSON file
            LogWindowEvent("title_change", windowTitle, processName, processPath);
            
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
