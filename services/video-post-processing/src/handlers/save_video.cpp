#include "save_video.h"

#include <iostream>

#include "../../../../utils/redis/redis.h"
#include "../../../../utils/db/pg.h"

namespace handlers {

namespace {

/**
 * @brief Handles the request to save video data.
 * 
 * This function is responsible for handling the request to save video data. It receives a JSON payload
 * containing the Redis ID of the video and the YOLO result. It connects to Redis, retrieves the YOLO result
 * using the Redis ID, parses the result, and saves it to PostgreSQL. Finally, it deletes the data from Redis
 * and sends a response indicating the success or failure of the operation.
 * 
 * @param req The HTTP request object.
 * @param res The HTTP response object.
 */
void SaveVideoHandler(const crow::request& req, crow::response& res) {
    auto body = crow::json::load(req.body);
    if (!body) {
        res.code = 400;
        res.write("Invalid JSON");
        res.end();
        return;
    }

    std::string redis_id = body["redis_id"].s();

    // Connect to Redis
    redisContext *redis_conn = redis_utils::RedisConnect("127.0.0.1", 6379);
    if (redis_conn == nullptr) {
        res.code = 500;
        res.write("Redis connection error");
        res.end();
        return;
    }

    // Get YOLO result from Redis
    redisReply *reply = redis_utils::RedisGetByKey(redis_conn, "GET yolo_response:%s", redis_id.c_str());
    if (reply == nullptr) {
        res.code = 500;
        res.write("Failed to get data from Redis");
        res.end();
        return;
    }

    // Parse YOLO result
    auto yolo_result = crow::json::load(reply->str);
    freeReplyObject(reply);

    // Save to PostgreSQL
    bool success = utils::db::SaveAnalysisResult(redis_id, yolo_result);
    if (!success) {
        res.code = 500;
        res.write("Failed to save data to PostgreSQL");
        res.end();
        return;
    }

    // Delete data from Redis
    redisCommand(redis_conn, "DEL yolo_response:%s", redis_id.c_str());
    redisFree(redis_conn);

    res.code = 200;
    res.write("Data saved successfully");
    res.end();
}

}

void BindSaveVideoHandler(crow::SimpleApp& app) {
    CROW_ROUTE(app, "/save_video").methods(crow::HTTPMethod::POST)(SaveVideoHandler);
}

} // namespace handlers
