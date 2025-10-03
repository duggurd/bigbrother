#include "main_window.h"
#include "imgui.h"
#include "time_utils.h"

namespace bigbrother {
namespace viewer {

MainWindow::MainWindow(ID3D11Device* device)
    : m_iconManager(device)
    , m_settingsWindow(m_filterManager)
    , m_timelineView(m_iconManager, m_filterManager)
{
    // Load initial data
    m_dataFilePath = m_sessionLoader.GetDefaultDataPath();
    ReloadSessions();
    
    // Set up file watcher for event-based updates
    SetupFileWatcher();
    
    // Set up deletion callback
    m_timelineView.SetDeleteSessionCallback([this](int sessionIndex) {
        RequestDeleteSession(sessionIndex);
    });
}

MainWindow::~MainWindow() {
    CleanupFileWatcher();
}

void MainWindow::Render() {
    // Check file watcher for changes
    if (m_fileWatcherEnabled) {
        CheckFileWatcher();
        
        // Reload if file changed
        if (m_shouldReload) {
            ReloadSessions();
            m_shouldReload = false;
        }
    }
    
    // Get IO for window sizing
    ImGuiIO& io = ImGui::GetIO();
    
    // Full-screen main window
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(io.DisplaySize);
    ImGui::Begin("BigBrother Session Viewer", nullptr, 
        ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | 
        ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_MenuBar);

    RenderMenuBar();
    
    // Timeline view
    m_timelineView.Render(m_sessions);

    ImGui::End();

    // Settings window (separate window)
    m_settingsWindow.Render();
    
    // Delete confirmation dialog
    RenderDeleteConfirmation();
    
    // Start session dialog
    RenderStartSessionDialog();
}

void MainWindow::ReloadSessions() {
    try {
        m_sessions = m_sessionLoader.LoadFromFile(m_dataFilePath);
    } catch (const std::exception& e) {
        // If loading fails, clear sessions to prevent crashes
        m_sessions.clear();
        // TODO: Show error message to user
    }
}

void MainWindow::RenderMenuBar() {
    if (ImGui::BeginMenuBar())
    {
        ImGui::Text("File: %s", m_dataFilePath.c_str());
        ImGui::Spacing();
        
        if (ImGui::Button("Reload"))
        {
            ReloadSessions();
        }
        
        ImGui::SameLine();
        
        // File watcher toggle
        if (ImGui::Checkbox("Auto-refresh", &m_fileWatcherEnabled))
        {
            if (m_fileWatcherEnabled) {
                SetupFileWatcher();
            } else {
                CleanupFileWatcher();
            }
        }
        
        // Show status indicator when file watcher is on
        if (m_fileWatcherEnabled) {
            ImGui::SameLine();
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.2f, 1.0f, 0.2f, 1.0f));
            ImGui::Text("[Live]");
            ImGui::PopStyleColor();
        }
        
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
        
        // Session recording controls
        if (m_sessionLogger.IsSessionActive())
        {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.2f, 1.0f, 0.2f, 1.0f)); // Green
            ImGui::Text("Recording...");
            ImGui::PopStyleColor();
            
            ImGui::SameLine();
            if (ImGui::Button("Stop Session"))
            {
                m_sessionLogger.StopSession();
                ReloadSessions(); // Reload to show the new session
            }
        }
        else
        {
            if (ImGui::Button("Start Session"))
            {
                m_showStartSessionDialog = true;
                m_sessionComment[0] = '\0'; // Clear comment
            }
        }
        
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
        
        if (ImGui::Button("Settings"))
        {
            m_settingsWindow.Toggle();
        }
        
        ImGui::EndMenuBar();
    }
}

void MainWindow::RequestDeleteSession(int sessionIndex) {
    m_deleteSessionIndex = sessionIndex;
    m_showDeleteConfirmation = true;
}

void MainWindow::RenderDeleteConfirmation() {
    if (!m_showDeleteConfirmation) return;
    
    if (m_deleteSessionIndex < 0 || m_deleteSessionIndex >= m_sessions.size()) {
        m_showDeleteConfirmation = false;
        return;
    }
    
    const auto& session = m_sessions[m_deleteSessionIndex];
    
    ImGui::OpenPopup("Delete Session?");
    
    // Center the modal
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    
    if (ImGui::BeginPopupModal("Delete Session?", &m_showDeleteConfirmation, ImGuiWindowFlags_AlwaysAutoResize))
    {
        std::string start_time = FormatTime(session.start_timestamp);
        std::string end_time = FormatTime(session.end_timestamp);
        long long duration = session.end_timestamp - session.start_timestamp;
        
        ImGui::Text("Delete Session %d?", m_deleteSessionIndex + 1);
        ImGui::Spacing();
        ImGui::Text("Time: %s - %s (%s)", start_time.c_str(), end_time.c_str(), FormatDuration(duration).c_str());
        ImGui::Text("Applications: %d", (int)session.applications.size());
        ImGui::Spacing();
        ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.5f, 1.0f), "This action cannot be undone!");
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
        
        if (ImGui::Button("Delete", ImVec2(120, 0)))
        {
            // Perform deletion
            if (m_sessionLoader.DeleteSession(m_sessions, m_deleteSessionIndex))
            {
                // Save updated sessions to file
                if (m_sessionLoader.SaveToFile(m_dataFilePath, m_sessions))
                {
                    // Successfully deleted and saved
                }
            }
            m_showDeleteConfirmation = false;
            ImGui::CloseCurrentPopup();
        }
        
        ImGui::SetItemDefaultFocus();
        ImGui::SameLine();
        
        if (ImGui::Button("Cancel", ImVec2(120, 0)))
        {
            m_showDeleteConfirmation = false;
            ImGui::CloseCurrentPopup();
        }
        
        ImGui::EndPopup();
    }
}

void MainWindow::RenderStartSessionDialog() {
    if (!m_showStartSessionDialog) return;
    
    ImGui::OpenPopup("Start Session");
    
    // Center the modal
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    
    if (ImGui::BeginPopupModal("Start Session", &m_showStartSessionDialog, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::Text("What are you doing?");
        ImGui::Spacing();
        
        // Auto-focus the text input when dialog opens
        if (ImGui::IsWindowAppearing()) {
            ImGui::SetKeyboardFocusHere();
        }
        
        bool enter_pressed = ImGui::InputText("##comment", m_sessionComment, sizeof(m_sessionComment), 
            ImGuiInputTextFlags_EnterReturnsTrue);
        
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
        
        if (ImGui::Button("Start", ImVec2(120, 0)) || enter_pressed)
        {
            // Start session with the comment
            std::string comment(m_sessionComment);
            if (m_sessionLogger.StartSession(comment))
            {
                // Session started successfully
            }
            m_showStartSessionDialog = false;
            ImGui::CloseCurrentPopup();
        }
        
        ImGui::SetItemDefaultFocus();
        ImGui::SameLine();
        
        if (ImGui::Button("Cancel", ImVec2(120, 0)))
        {
            m_showStartSessionDialog = false;
            ImGui::CloseCurrentPopup();
        }
        
        ImGui::EndPopup();
    }
}

void MainWindow::SetupFileWatcher() {
    CleanupFileWatcher(); // Clean up any existing watcher
    
    // Extract directory path from file path
    size_t lastSlash = m_dataFilePath.find_last_of("\\/");
    if (lastSlash == std::string::npos) {
        return; // Invalid path
    }
    
    std::string directoryPath = m_dataFilePath.substr(0, lastSlash);
    
    // Set up directory change notification
    m_fileWatcherHandle = FindFirstChangeNotificationA(
        directoryPath.c_str(),
        FALSE,  // Don't watch subtree
        FILE_NOTIFY_CHANGE_LAST_WRITE | FILE_NOTIFY_CHANGE_SIZE
    );
    
    if (m_fileWatcherHandle == INVALID_HANDLE_VALUE) {
        // Failed to create watcher, disable it
        m_fileWatcherEnabled = false;
    }
}

void MainWindow::CheckFileWatcher() {
    if (m_fileWatcherHandle == INVALID_HANDLE_VALUE) {
        return;
    }
    
    // Check if notification was signaled (non-blocking)
    DWORD result = WaitForSingleObject(m_fileWatcherHandle, 0);
    
    if (result == WAIT_OBJECT_0) {
        // Directory changed, mark for reload
        m_shouldReload = true;
        
        // Reset the notification for next change
        FindNextChangeNotification(m_fileWatcherHandle);
    }
}

void MainWindow::CleanupFileWatcher() {
    if (m_fileWatcherHandle != INVALID_HANDLE_VALUE) {
        FindCloseChangeNotification(m_fileWatcherHandle);
        m_fileWatcherHandle = INVALID_HANDLE_VALUE;
    }
}

} // namespace viewer
} // namespace bigbrother
