#pragma once

#include <vector>
#include <functional>
#include <set>
#include <string>
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
    
    // State tracking for tree node expansion
    std::set<std::string> m_closedSessions;      // Session IDs that are explicitly closed by user
    std::set<std::string> m_openApplications;    // Application IDs that are explicitly opened by user
    std::set<std::string> m_seenSessions;        // Sessions we've seen (to distinguish new from existing)
    std::set<std::string> m_seenApplications;    // Applications we've seen
    
    // Generate stable IDs for state tracking
    std::string GetSessionId(const Session& session, int sessionIndex) const;
    std::string GetApplicationId(const Session& session, const ApplicationFocusEvent& app, int appIndex) const;

    // Render a single session
    void RenderSession(const Session& session, int sessionIndex);
    
    // Render an application within a session
    void RenderApplication(const Session& session, const ApplicationFocusEvent& app, int appIndex);
    
    // Render tabs for an application
    void RenderTabs(const std::vector<TabInfo>& tabs);
    
    // Format time duration from milliseconds
    std::string FormatDurationMs(long long durationMs) const;
};

} // namespace viewer
} // namespace bigbrother
