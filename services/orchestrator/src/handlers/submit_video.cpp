#include "submit_video.h"

#include <filesystem>

#include "../redis/redis.h"

namespace handlers {

void BindSubmitVideoHandler(crow::SimpleApp& app) {
    CROW_ROUTE(app, "/submit_video").methods(crow::HTTPMethod::POST)
    ([](const crow::request& req){
        auto video_path = req.body;

        // Check if the video file exists
        if (!std::filesystem::exists(video_path)) {
            return crow::response(400, "Video file does not exist");
        }

        std::string id = redis_utils::GenerateUUID();
        requests::VideoRequest request = {id, video_path, requests::VideoStatus::Received};

        // Connect to redis
        redisContext *redis_conn = redis_utils::RedisConnect("127.0.0.1", 6379);
        if (redis_conn == nullptr) {
            return crow::response(500, "Redis connection error");
        }

        // Save request to redis
        redis_utils::RedisSaveVideoRequest(redis_conn, request);

        return crow::response(200, id);
    });
}

} // namespace handlers