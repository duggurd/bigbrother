#include "timeline_view.h"
#include "imgui.h"
#include "time_utils.h"
#include <map>
#include <algorithm>

namespace bigbrother {
namespace viewer {

TimelineView::TimelineView(IconManager& iconManager, FilterManager& filterManager)
    : m_iconManager(iconManager)
    , m_filterManager(filterManager)
{
}

TimelineView::~TimelineView() {
}

void TimelineView::Render(const std::vector<Session>& sessions) {
    ImGui::Text("Session Timeline");
    ImGui::Separator();
    
    if (ImGui::BeginChild("UnifiedTimeline", ImVec2(0, -30), true))
    {
        std::string lastDate = "";
        
        // Safety limit: prevent crashes from rendering too many sessions
        const size_t maxSessions = 1000;
        size_t sessionsToRender = (sessions.size() > maxSessions) ? maxSessions : sessions.size();
        
        if (sessions.size() > maxSessions) {
            ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f), 
                "Warning: Only showing first %zu of %zu sessions for performance", 
                maxSessions, sessions.size());
            ImGui::Separator();
        }
        
        for (int sessionIdx = 0; sessionIdx < sessionsToRender; sessionIdx++)
        {
            const auto& session = sessions[sessionIdx];
            
            // Check if we need to display a new date header
            std::string currentDate = FormatDate(session.start_timestamp);
            if (currentDate != lastDate)
            {
                if (sessionIdx > 0) {
                    ImGui::Spacing();
                    ImGui::Spacing();
                }
                
                // Display date header (non-expandable)
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.8f, 0.8f, 1.0f, 1.0f)); // Light blue
                ImGui::TextUnformatted(("--- " + FormatDateWithDay(session.start_timestamp) + " ---").c_str());
                ImGui::PopStyleColor();
                ImGui::Spacing();
                
                lastDate = currentDate;
            }
            
            RenderSession(session, sessionIdx);
            ImGui::Spacing();
        }
        
        if (sessions.empty())
        {
            ImGui::TextDisabled("No sessions recorded yet. Run the monitor to start tracking.");
        }
    }
    ImGui::EndChild();
    
    // Overall statistics bar at bottom
    ImGui::Separator();
    
    // Count total applications and tabs
    size_t totalApps = 0;
    size_t totalTabs = 0;
    for (const auto& session : sessions) {
        totalApps += session.applications.size();
        for (const auto& app : session.applications) {
            totalTabs += app.tabs.size();
        }
    }
    
    ImGui::Text("Total: %zu sessions | %zu applications | %zu tabs", 
                sessions.size(), totalApps, totalTabs);
}

void TimelineView::RenderSession(const Session& session, int sessionIndex) {
    ImGui::PushID(sessionIndex);
    
    // Calculate session duration
    long long session_duration = session.end_timestamp - session.start_timestamp;
    
    // Session header with timestamps and duration
    std::string start_time = FormatTime(session.start_timestamp);
    std::string end_time = FormatTime(session.end_timestamp);
    std::string session_header = "Session " + std::to_string(sessionIndex + 1) + 
                        "  (" + start_time + " - " + end_time + ", " + 
                        FormatDuration(session_duration) + ")";
    
    // Add comment if present
    if (!session.comment.empty()) {
        session_header += " - " + session.comment;
    }
    
    // Session tree node with distinct color
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.6f, 1.0f, 0.6f, 1.0f)); // Light green
    bool session_open = ImGui::TreeNodeEx(session_header.c_str(), ImGuiTreeNodeFlags_DefaultOpen);
    ImGui::PopStyleColor();
    
    // Right-click context menu for session
    if (ImGui::BeginPopupContextItem())
    {
        if (ImGui::MenuItem("Delete Session"))
        {
            if (m_deleteSessionCallback) {
                m_deleteSessionCallback(sessionIndex);
            }
        }
        ImGui::EndPopup();
    }
    
    if (session_open)
    {
        // Display applications for this session
        const size_t maxAppsPerSession = 500;
        size_t appsRendered = 0;
        
        for (int appIdx = 0; appIdx < session.applications.size(); appIdx++)
        {
            const auto& app = session.applications[appIdx];
            
            // Apply program filters
            if (m_filterManager.IsFiltered(app.process_name)) {
                continue; // Skip this app
            }
            
            // Safety limit per session
            if (appsRendered >= maxAppsPerSession) {
                ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f), 
                    "... %zu more applications (hidden for performance)", 
                    session.applications.size() - appIdx);
                break;
            }
            
            RenderApplication(app, appIdx);
            appsRendered++;
        }
        
        ImGui::TreePop();
    }
    
    ImGui::PopID();
}

void TimelineView::RenderApplication(const ApplicationFocusEvent& app, int appIndex) {
    ImGui::PushID(appIndex);
    
    // Application header with icon and total time
    std::string first_focus_time = FormatTime(app.first_focus_time);
    std::string last_focus_time = FormatTime(app.last_focus_time);
    std::string total_time = FormatDurationMs(app.total_time_spent_ms);
    
    std::string app_header = app.process_name + " - " + total_time + 
                           " (" + first_focus_time + " to " + last_focus_time + ")";
    
    // Get and display application icon
    ID3D11ShaderResourceView* icon = m_iconManager.GetIcon(app.process_path);
    if (icon) {
        ImGui::Image((void*)icon, ImVec2(16, 16));
        ImGui::SameLine();
    }
    
    // Application tree node with yellow color
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.6f, 1.0f)); // Light yellow
    bool app_open = ImGui::TreeNode(app_header.c_str());
    ImGui::PopStyleColor();
    
    // Right-click context menu
    if (ImGui::BeginPopupContextItem())
    {
        std::string menu_text = "Add '" + app.process_name + "' to filters";
        if (ImGui::MenuItem(menu_text.c_str()))
        {
            if (m_filterManager.AddFilter(app.process_name, true))
            {
                m_filterManager.SaveSettings();
            }
        }
        ImGui::EndPopup();
    }
    
    if (app_open)
    {
        ImGui::Text("Path: %s", app.process_path.c_str());
        
        RenderTabs(app.tabs);
        
        ImGui::TreePop();
    }
    
    ImGui::PopID();
}

void TimelineView::RenderTabs(const std::vector<TabInfo>& tabs) {
    if (tabs.empty()) {
        ImGui::Text("No tabs recorded");
        return;
    }
    
    ImGui::Spacing();
    
    // Sort tabs by time spent (longest first)
    std::vector<TabInfo> sortedTabs = tabs;
    std::sort(sortedTabs.begin(), sortedTabs.end(), 
        [](const TabInfo& a, const TabInfo& b) { 
            return a.total_time_spent_ms > b.total_time_spent_ms; 
        });
    
    ImGui::Text("Time per Tab:");
    ImGui::Indent();
    
    for (const auto& tab : sortedTabs)
    {
        // Format duration with fixed width for alignment
        std::string duration = FormatDurationMs(tab.total_time_spent_ms);
        // Pad duration to 8 characters for alignment
        while (duration.length() < 8) {
            duration = " " + duration;
        }
        
        std::string display = duration + " | " + tab.window_title;
        ImGui::Text("%s", display.c_str());
    }
    ImGui::Unindent();
}

std::string TimelineView::FormatDurationMs(long long durationMs) const {
    // Convert milliseconds to seconds
    long long durationSec = durationMs / 1000;
    
    // Use the existing FormatDuration function which expects seconds
    return FormatDuration(durationSec);
}

} // namespace viewer
} // namespace bigbrother
