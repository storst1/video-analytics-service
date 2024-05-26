#pragma once

#include <string>

#include <hiredis.h>

#ifdef _WIN32
#include <objbase.h>
#pragma comment(lib, "ole32.lib")
#else
#include <uuid/uuid.h>
#endif

#include "../requests/requests.h"

namespace redis_utils {

std::string GenerateUUID();
redisContext* RedisConnect(const std::string& ip, const std::size_t port);
redisReply* RedisGetByKey(redisContext *redis_conn, const char* format, ...);
void RedisSaveVideoRequest(redisContext *redis_conn, const requests::VideoRequest& request);

} // namespace redis_utils
