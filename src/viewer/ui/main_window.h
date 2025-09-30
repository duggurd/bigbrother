#pragma once

#include <vector>
#include "session_data.h"
#include "session_logger.h"
#include "ui/settings_window.h"
#include "ui/timeline_view.h"
#include "data/session_loader.h"
#include "data/filter_manager.h"
#include "graphics/icon_manager.h"

namespace bigbrother {
namespace viewer {

/**
 * @brief Main application window
 * 
 * Coordinates all UI components and manages application state.
 */
class MainWindow {
public:
    MainWindow(ID3D11Device* device);
    ~MainWindow();

    /**
     * @brief Render the main window and all child components
     */
    void Render();

    /**
     * @brief Reload session data from file
     */
    void ReloadSessions();

private:
    // Components
    SessionLogger m_sessionLogger;
    SessionLoader m_sessionLoader;
    FilterManager m_filterManager;
    IconManager m_iconManager;
    SettingsWindow m_settingsWindow;
    TimelineView m_timelineView;

    // Data
    std::vector<Session> m_sessions;
    std::string m_dataFilePath;

    // UI
    void RenderMenuBar();
};

} // namespace viewer
} // namespace bigbrother
