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
        
        for (int sessionIdx = 0; sessionIdx < sessions.size(); sessionIdx++)
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
    ImGui::Text("Total Sessions: %d", (int)sessions.size());
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
    
    // Session tree node with distinct color
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.6f, 1.0f, 0.6f, 1.0f)); // Light green
    bool session_open = ImGui::TreeNodeEx(session_header.c_str(), ImGuiTreeNodeFlags_DefaultOpen);
    ImGui::PopStyleColor();
    
    if (session_open)
    {
        // Display focus events for this session
        for (int eventIdx = 0; eventIdx < session.window_focus.size(); eventIdx++)
        {
            const auto& event = session.window_focus[eventIdx];
            
            // Apply program filters
            if (m_filterManager.IsFiltered(event.process_name)) {
                continue; // Skip this event
            }
            
            RenderFocusEvent(event, session, eventIdx);
        }
        
        ImGui::TreePop();
    }
    
    ImGui::PopID();
}

void TimelineView::RenderFocusEvent(const WindowFocusEvent& event, const Session& session, int eventIndex) {
    ImGui::PushID(eventIndex);
    
    // Calculate event duration
    long long event_duration = CalculateEventDuration(session, eventIndex);
    
    // Event header with icon
    std::string time_str = FormatTime(event.focus_timestamp);
    std::string event_header = time_str + " - " + event.process_name + " (" + FormatDuration(event_duration) + ")";
    
    // Get and display application icon
    ID3D11ShaderResourceView* icon = m_iconManager.GetIcon(event.process_path);
    if (icon) {
        ImGui::Image((void*)icon, ImVec2(16, 16));
        ImGui::SameLine();
    }
    
    // Event tree node with yellow color
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.6f, 1.0f)); // Light yellow
    bool event_open = ImGui::TreeNode(event_header.c_str());
    ImGui::PopStyleColor();
    
    // Right-click context menu
    if (ImGui::BeginPopupContextItem())
    {
        std::string menu_text = "Add '" + event.process_name + "' to filters";
        if (ImGui::MenuItem(menu_text.c_str()))
        {
            if (m_filterManager.AddFilter(event.process_name, true))
            {
                m_filterManager.SaveSettings();
            }
        }
        ImGui::EndPopup();
    }
    
    if (event_open)
    {
        ImGui::Text("Path: %s", event.process_path.c_str());
        
        RenderTitleChanges(event.title_changes, event, session, eventIndex);
        
        ImGui::TreePop();
    }
    
    ImGui::PopID();
}

void TimelineView::RenderTitleChanges(const std::vector<TitleChange>& titleChanges, 
                                      const WindowFocusEvent& event,
                                      const Session& session,
                                      int eventIndex) {
    if (titleChanges.empty()) {
        ImGui::Text("No title changes recorded");
        return;
    }
    
    ImGui::Spacing();
    
    // Calculate time spent on each unique title
    std::map<std::string, long long> titleDurations;
    
    for (size_t i = 0; i < titleChanges.size(); i++)
    {
        const auto& tc = titleChanges[i];
        long long duration = 0;
        
        // Calculate duration until next title change or end of focus event
        if (i < titleChanges.size() - 1) {
            duration = titleChanges[i + 1].timestamp - tc.timestamp;
        } else {
            // Last title change - duration until next focus event or session end
            if (eventIndex < session.window_focus.size() - 1) {
                duration = session.window_focus[eventIndex + 1].focus_timestamp - tc.timestamp;
            } else {
                duration = session.end_timestamp - tc.timestamp;
            }
        }
        
        // Aggregate durations for same title
        titleDurations[tc.title] += duration;
    }
    
    // Convert to vector for sorting
    std::vector<std::pair<std::string, long long>> sortedTitles(titleDurations.begin(), titleDurations.end());
    
    // Sort by duration (longest first)
    std::sort(sortedTitles.begin(), sortedTitles.end(), 
        [](const auto& a, const auto& b) { return a.second > b.second; });
    
    ImGui::Text("Time per Title:");
    ImGui::Indent();
    
    for (const auto& pair : sortedTitles)
    {
        // Format duration with fixed width for alignment
        std::string duration = FormatDuration(pair.second);
        // Pad duration to 5 characters for alignment
        while (duration.length() < 5) {
            duration = " " + duration;
        }
        
        std::string display = duration + " | " + pair.first;
        ImGui::Text("%s", display.c_str());
    }
    ImGui::Unindent();
}

long long TimelineView::CalculateEventDuration(const Session& session, int eventIndex) const {
    if (eventIndex < session.window_focus.size() - 1) {
        return session.window_focus[eventIndex + 1].focus_timestamp - 
               session.window_focus[eventIndex].focus_timestamp;
    } else {
        return session.end_timestamp - session.window_focus[eventIndex].focus_timestamp;
    }
}

} // namespace viewer
} // namespace bigbrother
