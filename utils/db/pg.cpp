#include "pg.h"
#include <pqxx/pqxx>
#include <filesystem>
#include <fstream>
#include <iostream>

namespace utils {
namespace db {

bool SaveAnalysisResult(const std::string& id, const crow::json::rvalue& analysis_result) {
    try {
        pqxx::connection C("dbname=yourdbname user=yourusername password=yourpassword hostaddr=127.0.0.1 port=5432");
        if (!C.is_open()) {
            std::cerr << "Can't open database" << std::endl;
            return false;
        }

        pqxx::work W(C);

        std::string query = "INSERT INTO analysis_results (id, result) VALUES (" +
                            W.quote(id) + ", " + W.quote(analysis_result.dump()) + ");";

        W.exec(query);
        W.commit();
        C.disconnect();
        return true;
    } catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
        return false;
    }
}

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
