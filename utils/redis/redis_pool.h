#pragma once

#include <vector>
#include <mutex>
#include <string>
#include <condition_variable>

#include <hiredis.h>

namespace redis {

class RedisPool {
public:
    static RedisPool& getInstance();

    RedisPool(const RedisPool&) = delete;
    RedisPool& operator=(const RedisPool&) = delete;

    redisContext* getConnection();
    void releaseConnection(redisContext* connection);

private:
    RedisPool(const std::string& host, int port, int poolSize);
    ~RedisPool();

    void initializePool();
    void releasePool();

    std::string host_;
    int port_;
    int poolSize_;
    std::vector<redisContext*> pool_;
    std::mutex mutex_;
    std::condition_variable cv_;
};

} // namespace redis