#pragma once

#include <string>
#include <vector>
#include "session_data.h"
#include "json.hpp"

namespace bigbrother {
namespace viewer {

/**
 * @brief Loads and parses session data from JSON files
 */
class SessionLoader {
public:
    SessionLoader();
    ~SessionLoader();

    /**
     * @brief Load sessions from JSON file
     * @param filePath Path to focus_log.json
     * @return Vector of parsed sessions
     */
    std::vector<Session> LoadFromFile(const std::string& filePath);

    /**
     * @brief Get default data file path
     * @return Path to %APPDATA%\BigBrother\focus_log.json
     */
    std::string GetDefaultDataPath() const;

    /**
     * @brief Save sessions to JSON file
     * @param filePath Path to focus_log.json
     * @param sessions Vector of sessions to save
     * @return true if saved successfully
     */
    bool SaveToFile(const std::string& filePath, const std::vector<Session>& sessions);

    /**
     * @brief Delete a session by index
     * @param sessions Vector of sessions
     * @param sessionIndex Index of session to delete
     * @return true if deleted successfully
     */
    bool DeleteSession(std::vector<Session>& sessions, int sessionIndex);

private:
    // Helper to parse a single session from JSON
    Session ParseSession(const nlohmann::json& sessionJson);
    
    // Helper to parse a window focus event
    WindowFocusEvent ParseFocusEvent(const nlohmann::json& eventJson);
};

} // namespace viewer
} // namespace bigbrother
