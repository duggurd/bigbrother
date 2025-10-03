#pragma once

#include <vector>
#include <functional>
#include "session_data.h"
#include "graphics/icon_manager.h"
#include "data/filter_manager.h"

namespace bigbrother {
namespace viewer {

/**
 * @brief Timeline view UI component
 * 
 * Renders the session timeline with applications and tabs,
 * grouped by date, with icons and filtering.
 */
class TimelineView {
public:
    TimelineView(IconManager& iconManager, FilterManager& filterManager);
    ~TimelineView();

    /**
     * @brief Render the timeline view
     * @param sessions List of sessions to display
     */
    void Render(const std::vector<Session>& sessions);

    /**
     * @brief Set callback for session deletion requests
     * @param callback Function to call when user wants to delete a session
     */
    void SetDeleteSessionCallback(std::function<void(int)> callback) {
        m_deleteSessionCallback = callback;
    }

private:
    IconManager& m_iconManager;
    FilterManager& m_filterManager;
    std::function<void(int)> m_deleteSessionCallback;

    // Render a single session
    void RenderSession(const Session& session, int sessionIndex);
    
    // Render an application within a session
    void RenderApplication(const ApplicationFocusEvent& app, int appIndex);
    
    // Render tabs for an application
    void RenderTabs(const std::vector<TabInfo>& tabs);
    
    // Format time duration from milliseconds
    std::string FormatDurationMs(long long durationMs) const;
};

} // namespace viewer
} // namespace bigbrother
