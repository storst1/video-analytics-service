#pragma once

#include <iostream>
#include <vector>
#include <mutex>
#include <queue>

#include <pqxx/pqxx>

#include "../cfg/global_config.h"

namespace utils {
namespace db {

class PgPool {
public:
    static PgPool& getInstance();

    void operate(const char* query);

private:
    PgPool();
    ~PgPool();

    PgPool(const PgPool&) = delete;
    PgPool& operator=(const PgPool&) = delete;

    pqxx::connection* getConnection();
    void connect();
    void disconnect();
    void releaseConnection(pqxx::connection* conn);

    std::queue<pqxx::connection*> connections_;
    std::mutex mutex_;
    int poolSize_ = 5;
};

} // namespace db
} // namespace utils

