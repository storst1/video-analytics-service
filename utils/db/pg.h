#pragma once

#include <crow/json.h>
#include <string>

namespace utils {
namespace db {

bool SaveAnalysisResult(const std::string& id, const crow::json::rvalue& analysis_result);
void ApplyMigrations(const std::string& connection_str, const std::string& migrations_dir);

} // namespace db
} // namespace utils
