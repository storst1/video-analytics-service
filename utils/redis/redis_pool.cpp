#include "redis_pool.h"
#include <iostream>

#include "../cfg/global_config.h"

namespace redis {

RedisPool& RedisPool::getInstance() {
    constexpr std::size_t poolSize = 5;
    const auto& config = cfg::GlobalConfig::getInstance();
    const auto& redisConfig = config.getRedis();
    static RedisPool instance(redisConfig.host, redisConfig.port, poolSize);
    return instance;
}

RedisPool::RedisPool(const std::string& host, int port, int poolSize)
    : host_(host), port_(port), poolSize_(poolSize) {
    initializePool();
}

RedisPool::~RedisPool() {
    releasePool();
}

redisContext* RedisPool::getConnection() {
    std::unique_lock<std::mutex> lock(mutex_);
    while (pool_.empty()) {
        cv_.wait(lock);
    }
    redisContext* connection = pool_.back();
    pool_.pop_back();
    return connection;
}

void RedisPool::releaseConnection(redisContext* connection) {
    std::unique_lock<std::mutex> lock(mutex_);
    pool_.push_back(connection);
    cv_.notify_one();
}

void RedisPool::initializePool() {
    for (int i = 0; i < poolSize_; ++i) {
        redisContext* connection = redisConnect(host_.c_str(), port_);
        if (connection != nullptr && connection->err == 0) {
            pool_.push_back(connection);
        } else {
            std::cerr << "Failed to create Redis connection: " 
                      << (connection != nullptr ? connection->errstr : "Unknown error") 
                      << std::endl;
        }
    }
}

void RedisPool::releasePool() {
    for (redisContext* connection : pool_) {
        redisFree(connection);
    }
    pool_.clear();
}

} // namespace redis