#pragma once

#include <string>
#include <vector>

namespace bigbrother {
namespace viewer {

/**
 * @brief Program filter configuration
 */
struct ProgramFilter {
    std::string program_name;
    bool enabled;
};

/**
 * @brief Manages program filtering and settings persistence
 * 
 * Handles adding/removing/toggling program filters and
 * saves/loads filter settings from JSON file.
 */
class FilterManager {
public:
    FilterManager();
    ~FilterManager();

    /**
     * @brief Add a new program filter
     * @param programName Name of program to filter (e.g., "explorer.exe")
     * @param enabled Whether filter is active
     * @return true if added, false if already exists
     */
    bool AddFilter(const std::string& programName, bool enabled = true);

    /**
     * @brief Remove a program filter by index
     * @param index Index in filter list
     */
    void RemoveFilter(size_t index);

    /**
     * @brief Toggle filter enabled state
     * @param index Index in filter list
     */
    void ToggleFilter(size_t index);

    /**
     * @brief Check if a program is filtered
     * @param programName Name to check
     * @return true if program should be hidden
     */
    bool IsFiltered(const std::string& programName) const;

    /**
     * @brief Get all filters
     */
    const std::vector<ProgramFilter>& GetFilters() const { return m_filters; }

    /**
     * @brief Get mutable filters (for ImGui)
     */
    std::vector<ProgramFilter>& GetFilters() { return m_filters; }

    /**
     * @brief Save filters to settings file
     */
    void SaveSettings();

    /**
     * @brief Load filters from settings file
     */
    void LoadSettings();

private:
    std::vector<ProgramFilter> m_filters;
    std::string GetSettingsFilePath() const;
};

} // namespace viewer
} // namespace bigbrother
