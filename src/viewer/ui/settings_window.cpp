#include "settings_window.h"
#include "imgui.h"

namespace bigbrother {
namespace viewer {

SettingsWindow::SettingsWindow(FilterManager& filterManager)
    : m_filterManager(filterManager)
    , m_isVisible(false)
{
    m_newProgramName[0] = '\0';
}

SettingsWindow::~SettingsWindow() {
}

void SettingsWindow::Render() {
    if (!m_isVisible) return;
    
    ImGui::SetNextWindowSize(ImVec2(500, 400), ImGuiCond_FirstUseEver);
    ImGui::Begin("Settings", &m_isVisible);
    
    ImGui::Text("Program Filters");
    ImGui::Separator();
    ImGui::Spacing();
    
    ImGui::TextWrapped("Hide focus events from specific programs. Check the box to enable filtering for that program.");
    ImGui::Spacing();
    
    // Add new program section
    ImGui::Text("Add Program:");
    ImGui::PushItemWidth(300);
    ImGui::InputText("##newprogram", m_newProgramName, sizeof(m_newProgramName));
    ImGui::PopItemWidth();
    ImGui::SameLine();
    if (ImGui::Button("Add"))
    {
        std::string programName = std::string(m_newProgramName);
        if (!programName.empty())
        {
            if (m_filterManager.AddFilter(programName, true)) {
                // Clear input field on success
                m_newProgramName[0] = '\0';
            }
        }
    }
    
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();
    
    // List of filtered programs
    ImGui::Text("Filtered Programs:");
    ImGui::BeginChild("FilterList", ImVec2(0, 200), true);
    
    // Track which filter to delete (if any)
    int deleteIndex = -1;
    
    auto& filters = m_filterManager.GetFilters();
    for (size_t i = 0; i < filters.size(); i++)
    {
        ImGui::PushID((int)i);
        
        // Checkbox to enable/disable filter
        bool wasEnabled = filters[i].enabled;
        ImGui::Checkbox("##enabled", &filters[i].enabled);
        if (wasEnabled != filters[i].enabled) {
            m_filterManager.SaveSettings(); // Save when checkbox changes
        }
        ImGui::SameLine();
        
        // Program name
        ImGui::Text("%s", filters[i].program_name.c_str());
        ImGui::SameLine();
        
        // Delete button (right-aligned)
        float buttonX = ImGui::GetContentRegionAvail().x - 60;
        if (buttonX > 0) {
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + buttonX);
        }
        if (ImGui::Button("Delete"))
        {
            deleteIndex = (int)i;
        }
        
        ImGui::PopID();
    }
    
    // Delete filter if requested
    if (deleteIndex >= 0) {
        m_filterManager.RemoveFilter(deleteIndex);
    }
    
    if (filters.empty())
    {
        ImGui::TextDisabled("No programs filtered. Add one above.");
    }
    
    ImGui::EndChild();
    
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();
    
    if (ImGui::Button("Close"))
    {
        m_isVisible = false;
    }
    
    ImGui::End();
}

} // namespace viewer
} // namespace bigbrother
