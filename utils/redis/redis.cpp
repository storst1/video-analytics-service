#include "redis.h"

#include <iostream>
#include <cstdio> // for snprintf

namespace redis_utils {

/**
 * Generates a universally unique identifier (UUID) string.
 *
 * On Windows, the function uses the CoCreateGuid function to generate a GUID and formats it into a UUID string.
 * On other platforms, the function uses the uuid_generate and uuid_unparse functions to generate and convert a UUID to a string.
 *
 * @return A string representing the generated UUID.
 */
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

/**
 * @brief Connects to a Redis server.
 * 
 * This function establishes a connection to a Redis server using the specified IP address and port.
 * 
 * @param ip The IP address of the Redis server.
 * @param port The port number of the Redis server.
 * @return A pointer to the redisContext structure representing the connection, or nullptr if an error occurred.
 */
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

/**
 * Retrieves data from Redis based on the provided format and arguments.
 *
 * @param redis_conn A pointer to the Redis connection.
 * @param format The format string for the Redis command.
 * @param ... Additional arguments for the format string.
 * @return A pointer to the Redis reply containing the requested data, or nullptr if the reply is NULL or empty.
 */
redisReply* RedisGetByKey(redisContext *redis_conn, const char* format, ...) {
    // Get request from Redis
    va_list args;
    va_start(args, format);
    redisReply *reply = (redisReply*)redisvCommand(redis_conn, format, args);
    va_end(args);

    if (reply == NULL || reply->type == REDIS_REPLY_NIL) {
        if (reply) {
            freeReplyObject(reply);
        }
        redisFree(redis_conn);
        return nullptr;
    }
    return reply;
}

/**
 * Saves a video request to Redis.
 * 
 * @param redis_conn The Redis connection.
 * @param request The video request to be saved.
 */
void RedisSaveVideoRequest(redisContext *redis_conn, const requests::VideoRequest& request) {
    redisCommand(redis_conn, "HMSET request:%s id %s path %s status %s", 
                 request.id.c_str(), request.id.c_str(), request.path.c_str(), 
                 requests::VideoStatusToString(request.status).c_str());
}

/**
 * @brief Updates the status of a video in Redis.
 *
 * This function updates the status of a video in Redis by executing an HSET command.
 * The video status is stored as a string in the Redis hash with the specified key.
 * The status is updated only if the current status is not VideoStatus::Stopped.
 *
 * @param redis_conn A pointer to the Redis connection.
 * @param key The key of the Redis hash where the video status is stored.
 * @param new_status The new status of the video.
 */
void RedisUpdateVideoStatus(redisContext *redis_conn, const std::string& key, requests::VideoStatus new_status) {
    if (redis_conn == nullptr) {
        std::cerr << "Redis connection is null" << std::endl;
        return;
    }

    // Get the current status from Redis
    std::optional<requests::VideoStatus> current_status = RedisGetRequestVideoStatus(redis_conn, key);
    if (!current_status) {
        std::cerr << "Failed to get current status from Redis" << std::endl;
        return;
    }

    // Check if the current status is VideoStatus::Stopped
    if (*current_status == requests::VideoStatus::Stopped) {
        std::cout << "Cannot update status. Current status is VideoStatus::Stopped" << std::endl;
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

/**
 * @brief Saves a JSON response to Redis.
 *
 * This function takes a Redis connection, a key, and a JSON response and saves the JSON response as a string in Redis.
 *
 * @param redis_conn A pointer to the Redis connection.
 * @param key The key under which the JSON response will be saved in Redis.
 * @param json_response The JSON response to be saved in Redis.
 */
void RedisSaveJsonResponse(redisContext *redis_conn, const std::string& key, const crow::json::wvalue& json_response) {
    std::string json_str = json_response.dump();
    redisCommand(redis_conn, "SET %s %s", key.c_str(), json_str.c_str());
}

/**
 * Saves the YOLO response in Redis.
 *
 * @param redis_conn The Redis connection.
 * @param id The identifier for the YOLO response.
 * @param json_response The JSON response to be saved.
 */
void RedisSaveYoloResponse(redisContext *redis_conn, const std::string& id, const crow::json::wvalue& json_response) {
    std::string json_str = json_response.dump();
    std::string key = "yolo_response:" + id;
    redisCommand(redis_conn, "SET %s %s", key.c_str(), json_str.c_str());
}

/**
 * Retrieves the status of a video request from Redis.
 *
 * @param redis_conn A pointer to the Redis connection.
 * @param key The key of the Redis hash where the video request is stored.
 * @return The status of the video request, or an empty string if the request is not found or an error occurs.
 */
std::optional<requests::VideoStatus> RedisGetRequestVideoStatus(redisContext *redis_conn, const std::string& key) {
    if (redis_conn == nullptr) {
        std::cerr << "Redis connection is null" << std::endl;
        return std::nullopt;
    }

    const std::string command = "HGET request:" + key + " status";
    redisReply *reply = static_cast<redisReply*>(redisCommand(redis_conn, command.c_str()));
    if (reply == nullptr) {
        std::cerr << "Failed to execute command: " << command << std::endl;
        return std::nullopt;
    }

    std::string status;
    if (reply->type == REDIS_REPLY_STRING) {
        status = reply->str;
    }

    freeReplyObject(reply);
    return requests::StringToVideoStatus(status);
}

} // namespace redis_utils