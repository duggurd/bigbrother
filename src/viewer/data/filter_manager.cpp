#include "filter_manager.h"
#include <windows.h>
#include <shlobj.h>
#include <fstream>
#include "json.hpp"

using json = nlohmann::json;

namespace bigbrother {
namespace viewer {

FilterManager::FilterManager() {
    LoadSettings();
}

FilterManager::~FilterManager() {
    // Settings are saved on each modification, no need to save here
}

bool FilterManager::AddFilter(const std::string& programName, bool enabled) {
    // Check if already exists
    for (const auto& filter : m_filters) {
        if (filter.program_name == programName) {
            return false; // Already exists
        }
    }
    
    ProgramFilter newFilter;
    newFilter.program_name = programName;
    newFilter.enabled = enabled;
    m_filters.push_back(newFilter);
    
    SaveSettings();
    return true;
}

void FilterManager::RemoveFilter(size_t index) {
    if (index < m_filters.size()) {
        m_filters.erase(m_filters.begin() + index);
        SaveSettings();
    }
}

void FilterManager::ToggleFilter(size_t index) {
    if (index < m_filters.size()) {
        m_filters[index].enabled = !m_filters[index].enabled;
        SaveSettings();
    }
}

bool FilterManager::IsFiltered(const std::string& programName) const {
    for (const auto& filter : m_filters) {
        if (filter.enabled && filter.program_name == programName) {
            return true;
        }
    }
    return false;
}

void FilterManager::SaveSettings() {
    json settings;
    settings["program_filters"] = json::array();
    
    for (const auto& filter : m_filters) {
        json filterJson;
        filterJson["program_name"] = filter.program_name;
        filterJson["enabled"] = filter.enabled;
        settings["program_filters"].push_back(filterJson);
    }
    
    std::string settingsPath = GetSettingsFilePath();
    std::ofstream outFile(settingsPath);
    if (outFile.is_open()) {
        outFile << settings.dump(2) << std::endl;
        outFile.close();
    }
}

void FilterManager::LoadSettings() {
    std::string settingsPath = GetSettingsFilePath();
    std::ifstream inFile(settingsPath);
    
    if (!inFile.is_open()) {
        return; // No settings file yet, use defaults
    }
    
    try {
        json settings;
        inFile >> settings;
        inFile.close();
        
        if (settings.contains("program_filters") && settings["program_filters"].is_array()) {
            m_filters.clear();
            
            for (const auto& filterJson : settings["program_filters"]) {
                ProgramFilter filter;
                filter.program_name = filterJson.value("program_name", "");
                filter.enabled = filterJson.value("enabled", true);
                
                if (!filter.program_name.empty()) {
                    m_filters.push_back(filter);
                }
            }
        }
    } catch (const json::exception& e) {
        // Error parsing settings, use defaults
    }
}

std::string FilterManager::GetSettingsFilePath() const {
    char path[MAX_PATH];
    if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_APPDATA, NULL, 0, path))) {
        return std::string(path) + "\\BigBrother\\viewer_settings.json";
    }
    return "viewer_settings.json";
}

} // namespace viewer
} // namespace bigbrother
