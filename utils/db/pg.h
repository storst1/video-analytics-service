#pragma once

#include <string>
#include <optional>

#include <crow/json.h>

namespace utils {
namespace db {

bool SaveAnalysisResult(const std::string& id, const crow::json::wvalue& analysis_result);
bool SaveRequestOnReceive(const std::string& id);
bool UpdateVideoStatus(const std::string& id, const std::string& video_status);
std::optional<std::string> GetVideoStatus(const std::string& id);
std::optional<crow::json::wvalue> GetAnalysisResult(const std::string& id);
void ApplyMigrations(const std::string& connection_str, const std::string& migrations_dir);

} // namespace db
} // namespace utils
