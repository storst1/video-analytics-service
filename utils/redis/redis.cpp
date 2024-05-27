#include "redis.h"

#include <iostream>

namespace redis_utils {

std::string GenerateUUID() {
    #ifdef _WIN32
    GUID guid;
    CoCreateGuid(&guid);

    // Buffer to hold the UUID string
    char uuid_str[37];
    // Format the GUID into a UUID string
    snprintf(uuid_str, sizeof(uuid_str),
             "%08x-%04x-%04x-%04x-%012llx",
             (unsigned int)guid.Data1, guid.Data2, guid.Data3,
             (guid.Data4[0] << 8) | guid.Data4[1],
             (unsigned long long)((guid.Data4[2] << 40) | (guid.Data4[3] << 32) |
                                  (guid.Data4[4] << 24) | (guid.Data4[5] << 16) |
                                  (guid.Data4[6] << 8) | guid.Data4[7]));
    // Return the UUID string
    return std::string(uuid_str);
    #else
    uuid_t uuid;
    uuid_generate(uuid);
    char uuid_str[37];
    // Convert the UUID to a string
    uuid_unparse(uuid, uuid_str);
    return std::string(uuid_str);
    #endif
}

redisContext* RedisConnect(const std::string& ip, const std::size_t port) {
    redisContext *c = redisConnect(ip.c_str(), port);
    if (c == nullptr || c->err) {
        if (c) {
            std::cerr << "Error: " << c->errstr << std::endl;
            redisFree(c);
        } else {
            std::cerr << "Can't allocate redis context" << std::endl;
        }
        return nullptr;
    }
    return c;
}

redisReply* RedisGetByKey(redisContext *redis_conn, const char* format, ...) {
    // Получение данных запроса из Redis
    redisReply *reply = static_cast<redisReply*>(redisCommand(redis_conn, format));
    if (reply == NULL || reply->type == REDIS_REPLY_NIL) {
        if (reply) {
            freeReplyObject(reply);
        }
        redisFree(redis_conn);
        return nullptr;
    }
    return reply;
}

void RedisSaveVideoRequest(redisContext *redis_conn, const requests::VideoRequest& request) {
    redisCommand(redis_conn, "HMSET request:%s id %s path %s status %s", 
                     request.id.c_str(), request.id.c_str(), request.path.c_str(), 
                     requests::VideoStatusToString(request.status).c_str());
    redisFree(redis_conn);
}

void RedisUpdateVideoStatus(redisContext *redis_conn, const std::string& key, requests::VideoStatus new_status) {
    if (redis_conn == nullptr) {
        std::cerr << "Redis connection is null" << std::endl;
        return;
    }

    const std::string command = "HSET request:" + key + " status " + requests::VideoStatusToString(new_status);
    redisReply *reply = static_cast<redisReply*>(redisCommand(redis_conn, command.c_str()));
    if (reply == nullptr) {
        std::cerr << "Failed to execute command: " << command << std::endl;
        return;
    }

    freeReplyObject(reply);
}

} // namespace redis_utils