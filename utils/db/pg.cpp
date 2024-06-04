#include "pg.h"

#include <filesystem>
#include <fstream>
#include <iostream>

#include <pqxx/pqxx>
#include <crow/returnable.h>

#include "../cfg/global_config.h"


namespace utils {
namespace db {

/**
 * Saves the analysis result to the database.
 * 
 * @param id The ID of the analysis result.
 * @param analysis_result The analysis result to be saved.
 * @return True if the analysis result is successfully saved, false otherwise.
 */
bool SaveAnalysisResult(const std::string& id, const crow::json::wvalue& analysis_result) {
    try {
        const auto& config = cfg::GlobalConfig::getInstance();
        const auto& pg_db = config.getPgDatabaseConfig();
        pqxx::connection C(pg_db.getConnectionString());
        if (!C.is_open()) {
            std::cerr << "Can't open database" << std::endl;
            return false;
        }

        pqxx::work W(C);
        std::string query = "UPDATE analysis_results SET result = " +
                    W.quote(analysis_result.dump()) + ", video_status = " + W.quote("Finished") +
                    " WHERE id = " + W.quote(id) + ";";

        W.exec(query);
        W.commit();
        C.close();
        return true;
    } catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
        return false;
    }
}

/**
 * Saves a new request to the database.
 * 
 * @param id The ID of the request.
 * @return True if the request is successfully saved, false otherwise.
 */
bool SaveRequestOnReceive(const std::string& id) {
    try {
        const auto& config = cfg::GlobalConfig::getInstance();
        const auto& pg_db = config.getPgDatabaseConfig();
        pqxx::connection C(pg_db.getConnectionString());
        if (!C.is_open()) {
            std::cerr << "Can't open database" << std::endl;
            return false;
        }

        pqxx::work W(C);
        std::string query = "INSERT INTO analysis_results (id, result, video_status) VALUES (" +
                            W.quote(id) + ", " + W.quote("{}") + ", " + W.quote("Received") + ");";

        W.exec(query);
        W.commit();
        C.close();
        return true;
    } catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
        return false;
    }
}

/**
 * Updates the video_status for the given ID in the PostgreSQL database.
 * 
 * @param id The ID of the video.
 * @param video_status The new video status.
 * @return True if the video_status is successfully updated, false otherwise.
 */
bool UpdateVideoStatus(const std::string& id, const std::string& video_status) {
    try {
        const auto& config = cfg::GlobalConfig::getInstance();
        const auto& pg_db = config.getPgDatabaseConfig();
        pqxx::connection C(pg_db.getConnectionString());
        if (!C.is_open()) {
            std::cerr << "Can't open database" << std::endl;
            return false;
        }

        pqxx::work W(C);

        std::string query = "UPDATE analysis_results SET video_status = " +
                            W.quote(video_status) + " WHERE id = " + W.quote(id) + ";";

        W.exec(query);
        W.commit();
        C.close();
        return true;
    } catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
        return false;
    }
}

/**
 * Retrieves the video_status for the given ID from the PostgreSQL database.
 * 
 * @param id The ID of the video.
 * @return The video_status if found, std::nullopt otherwise.
 */
std::optional<std::string> GetVideoStatus(const std::string& id) {
    try {
        const auto& config = cfg::GlobalConfig::getInstance();
        const auto& pg_db = config.getPgDatabaseConfig();
        pqxx::connection C(pg_db.getConnectionString());
        if (!C.is_open()) {
            std::cerr << "Can't open database" << std::endl;
            return std::nullopt;
        }

        pqxx::work W(C);

        std::string query = "SELECT video_status FROM analysis_results WHERE id = " + W.quote(id) + ";";

        pqxx::result result = W.exec(query);
        if (result.empty()) {
            return std::nullopt;
        }

        std::string video_status = result[0][0].as<std::string>();
        W.commit();
        C.close();
        return video_status;
    } catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
        return std::nullopt;
    }
}

/**
 * Retrieves the analysis result for the given ID from the PostgreSQL database.
 * 
 * @param id The ID of the analysis result.
 * @return The analysis result if found, std::nullopt otherwise.
 */
std::optional<crow::json::wvalue> GetAnalysisResult(const std::string& id) {
    try {
        const auto& config = cfg::GlobalConfig::getInstance();
        const auto& pg_db = config.getPgDatabaseConfig();
        pqxx::connection C(pg_db.getConnectionString());
        if (!C.is_open()) {
            std::cerr << "Can't open database" << std::endl;
            return std::nullopt;
        }

        pqxx::work W(C);

        std::string query = "SELECT result FROM analysis_results WHERE id = " + W.quote(id) + ";";

        pqxx::result result = W.exec(query);
        if (result.empty()) {
            return std::nullopt;
        }

        crow::json::wvalue analysis_result;
        analysis_result = crow::json::load(result[0][0].as<std::string>());
        W.commit();
        C.close();
        return analysis_result;
    } catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
        return std::nullopt;
    }
}

/**
 * Applies database migrations to the specified PostgreSQL database.
 *
 * @param connection_str The connection string for the PostgreSQL database.
 * @param migrations_dir The directory containing the migration files.
 */
void ApplyMigrations(const std::string& connection_str, const std::string& migrations_dir) {
    try {
        pqxx::connection C(connection_str);
        if (!C.is_open()) {
            std::cerr << "Can't open database" << std::endl;
            return;
        }

        pqxx::work txn(C);

        for (const auto& entry : std::filesystem::directory_iterator(migrations_dir)) {
            std::ifstream file(entry.path());
            if (!file.is_open()) {
                std::cerr << "Cannot open migration file: " << entry.path() << std::endl;
                continue;
            }

            std::string sql((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
            try {
                txn.exec(sql);
                std::cout << "Successfully applied migration: " << entry.path() << std::endl;
            } catch (const std::exception &e) {
                std::cerr << "Failed to apply migration: " << entry.path() << ". Error: " << e.what() << std::endl;
            }
        }

        txn.commit();
    } catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
    }
}

} // namespace db
} // namespace utils
