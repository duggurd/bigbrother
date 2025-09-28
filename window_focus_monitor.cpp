#include <windows.h>
#include <iostream>
#include <string>
#include <psapi.h>

// Global variable to store the hook handle
HWINEVENTHOOK g_hWinEventHook = NULL;

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
    // Check if this is a focus change event and it's for a window (not a child object)
    if (event == EVENT_SYSTEM_FOREGROUND && idObject == OBJID_WINDOW && hwnd != NULL) {
        std::string windowTitle = GetWindowTitle(hwnd);
        std::string processInfo = GetProcessInfo(hwnd);
        
        std::cout << "Focus changed to: " << windowTitle << std::endl;
        std::cout << "  Process: " << processInfo << std::endl;
        std::cout << "  ---" << std::endl;
    }
}

int main() {
    std::cout << "Window Focus Monitor Started" << std::endl;
    std::cout << "Press Ctrl+C to exit..." << std::endl;
    
    // Set up the hook to monitor foreground window changes
    g_hWinEventHook = SetWinEventHook(
        EVENT_SYSTEM_FOREGROUND,    // eventMin - we only want foreground changes
        EVENT_SYSTEM_FOREGROUND,    // eventMax - same as eventMin for single event
        NULL,                       // hmodWinEventProc - NULL for out-of-context hook
        WinEventProc,              // lpfnWinEventProc - our callback function
        0,                         // idProcess - 0 for all processes
        0,                         // idThread - 0 for all threads
        WINEVENT_OUTOFCONTEXT      // dwFlags - out-of-context hook
    );
    
    if (g_hWinEventHook == NULL) {
        std::cerr << "Failed to set up window event hook!" << std::endl;
        return 1;
    }
    
    std::cout << "Hook installed successfully. Monitoring window focus changes..." << std::endl;
    
    // Message loop to keep the program running
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    
    // Clean up
    if (g_hWinEventHook) {
        UnhookWinEvent(g_hWinEventHook);
    }
    
    return 0;
}
