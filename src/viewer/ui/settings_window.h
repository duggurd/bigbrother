#pragma once

#include "data/filter_manager.h"

namespace bigbrother {
namespace viewer {

/**
 * @brief Settings window UI
 * 
 * Handles rendering and interaction for the settings dialog,
 * including program filter management.
 */
class SettingsWindow {
public:
    SettingsWindow(FilterManager& filterManager);
    ~SettingsWindow();

    /**
     * @brief Render the settings window
     */
    void Render();

    /**
     * @brief Show/hide the settings window
     */
    void Show() { m_isVisible = true; }
    void Hide() { m_isVisible = false; }
    void Toggle() { m_isVisible = !m_isVisible; }

    /**
     * @brief Check if window is visible
     */
    bool IsVisible() const { return m_isVisible; }

private:
    FilterManager& m_filterManager;
    bool m_isVisible;
    char m_newProgramName[256];
};

} // namespace viewer
} // namespace bigbrother
