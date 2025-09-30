#include <windows.h>
#include <iostream>
#include "session_logger.h"

using namespace bigbrother;

// Global state
SessionLogger g_logger;
bool g_shouldExit = false;
bool g_shutdownInProgress = false;

// Console control handler for Ctrl+C
BOOL WINAPI ConsoleCtrlHandler(DWORD dwCtrlType) {
    switch (dwCtrlType) {
        case CTRL_C_EVENT:
        case CTRL_CLOSE_EVENT:
        case CTRL_BREAK_EVENT:
        case CTRL_LOGOFF_EVENT:
        case CTRL_SHUTDOWN_EVENT:
            if (g_shutdownInProgress) {
                return TRUE;
            }
            g_shutdownInProgress = true;
            
            std::cout << "\nShutting down gracefully..." << std::endl;
            g_shouldExit = true;
            g_logger.StopSession();
            
            std::cout << "Program exited successfully." << std::endl;
            ExitProcess(0);
        default:
            return FALSE;
    }
}

int main() {
    std::cout << "BigBrother Window Focus & Title Monitor Started" << std::endl;
    std::cout << "Press Ctrl+C to exit..." << std::endl;
    
    // Set up console control handler for graceful shutdown
    SetConsoleCtrlHandler(ConsoleCtrlHandler, TRUE);
    
    // Start monitoring session
    if (!g_logger.StartSession()) {
        std::cerr << "Failed to start session!" << std::endl;
        return 1;
    }
    
    std::cout << "Session started. Data will be saved to: " << g_logger.GetDataFilePath() << std::endl;
    std::cout << "Hooks installed successfully. Monitoring window focus and title changes..." << std::endl;
    
    // Message loop to keep the program running
    MSG msg;
    while (!g_shouldExit && GetMessage(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    
    // Clean up (fallback cleanup if not already done by Ctrl+C handler)
    if (!g_shutdownInProgress) {
        g_logger.StopSession();
        std::cout << "Program exited successfully." << std::endl;
    }
    
    return 0;
}
