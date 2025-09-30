#pragma once

#include <vector>
#include "session_data.h"
#include "graphics/icon_manager.h"
#include "data/filter_manager.h"

namespace bigbrother {
namespace viewer {

/**
 * @brief Timeline view UI component
 * 
 * Renders the session timeline with focus events,
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

private:
    IconManager& m_iconManager;
    FilterManager& m_filterManager;

    // Render a single session
    void RenderSession(const Session& session, int sessionIndex);
    
    // Render a focus event within a session
    void RenderFocusEvent(const WindowFocusEvent& event, const Session& session, int eventIndex);
    
    // Render title changes for an event
    void RenderTitleChanges(const std::vector<TitleChange>& titleChanges, 
                           const WindowFocusEvent& event,
                           const Session& session,
                           int eventIndex);
    
    // Calculate duration for an event
    long long CalculateEventDuration(const Session& session, int eventIndex) const;
};

} // namespace viewer
} // namespace bigbrother
