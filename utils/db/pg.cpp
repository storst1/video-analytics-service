#include "pg.h"

#include <filesystem>
#include <fstream>
#include <iostream>

#include <pqxx/pqxx>

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
bool SaveAnalysisResult(const std::string& id, const crow::json::rvalue& analysis_result) {
    try {
        const auto& config = cfg::GlobalConfig::getInstance();
        const auto& pg_db = config.getPgDatabaseConfig();
        pqxx::connection C(pg_db.getConnectionString());
        if (!C.is_open()) {
            std::cerr << "Can't open database" << std::endl;
            return false;
        }

        pqxx::work W(C);

        std::string query = "INSERT INTO analysis_results (id, result) VALUES (" +
                            W.quote(id) + ", " + W.quote(std::string(analysis_result)) + ");";

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
