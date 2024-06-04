#include "pg_pool.h"
#include "../cfg/global_config.h"
#include <iostream>

namespace utils {
namespace db {

PgPool& PgPool::getInstance() {
    static PgPool instance;
    return instance;
}

PgPool::PgPool() {
    connect();
}

PgPool::~PgPool() {
    disconnect();
}

void PgPool::operate(const char* query) {
    pqxx::connection* conn = getConnection();
    if (conn) {
        try {
            pqxx::work txn(*conn);
            txn.exec(query);
            txn.commit();
        } catch (const std::exception& e) {
            std::cerr << "Failed to execute query: " << e.what() << std::endl;
        }
        releaseConnection(conn);
    } else {
        std::cerr << "Not connected to PostgreSQL" << std::endl;
    }
}

pqxx::connection* PgPool::getConnection() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!connections_.empty()) {
        pqxx::connection* conn = connections_.front();
        connections_.pop();
        return conn;
    } else {
        std::cerr << "No available connections in the pool" << std::endl;
        return nullptr;
    }
}

void PgPool::connect() {
    std::lock_guard<std::mutex> lock(mutex_);
    const auto& pg_db = cfg::GlobalConfig::getInstance().getPgDatabaseConfig();
    for (int i = 0; i < poolSize_; i++) {
        try {
            pqxx::connection* conn = new pqxx::connection(pg_db.getConnectionString());
            if (conn->is_open()) {
                connections_.push(conn);
                std::cout << "Connected to PostgreSQL" << std::endl;
            } else {
                std::cerr << "Failed to open PostgreSQL connection" << std::endl;
                delete conn;
            }
        } catch (const std::exception& e) {
            std::cerr << "Failed to connect to PostgreSQL: " << e.what() << std::endl;
        }
    }
}

void PgPool::disconnect() {
    std::lock_guard<std::mutex> lock(mutex_);
    while (!connections_.empty()) {
        pqxx::connection* conn = connections_.front();
        connections_.pop();
        if (conn) {
            conn->close();
            delete conn;
            std::cout << "Disconnected from PostgreSQL" << std::endl;
        }
    }
}

void PgPool::releaseConnection(pqxx::connection* conn) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (conn) {
        connections_.push(conn);
    }
}

} // namespace db
} // namespace utils
