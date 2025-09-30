#include "main_window.h"
#include "imgui.h"

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
}

MainWindow::~MainWindow() {
}

void MainWindow::Render() {
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
}

void MainWindow::ReloadSessions() {
    m_sessions = m_sessionLoader.LoadFromFile(m_dataFilePath);
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
                if (m_sessionLogger.StartSession())
                {
                    // Session started successfully
                }
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

} // namespace viewer
} // namespace bigbrother
