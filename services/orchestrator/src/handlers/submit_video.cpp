#include "submit_video.h"
#include "../../utils/http/requests_chain.h"
#include <iostream>
#include <asio.hpp>
#include "../../utils/redis/redis.h"

namespace handlers {

namespace {

void OnYoloAnalyzeComplete(const crow::response& response) {
    if (response.code == 200) {
        std::cout << "YOLO analysis started successfully\n";
    } else {
        std::cout << "Failed to start YOLO analysis\n";
    }
}

void OnProcessVideoComplete(const crow::response& response, utils::http::RequestsChain& chain, const std::string& id) {
    if (response.code == 200) {
        std::string frames_folder = "./frames-" + id;
        chain.AddRequest("127.0.0.1", "8082", "/yolo_analyze_frames", frames_folder, OnYoloAnalyzeComplete);
        chain.Execute();
    } else {
        std::cout << "Failed to start video processing\n";
    }
}

void SubmitVideoHandler(const crow::request& req, crow::response& res) {
    auto video_path = req.body;
    std::string id = redis_utils::GenerateUUID();
    requests::VideoRequest video_request = {id, video_path, requests::VideoStatus::Received};

    // Connect to redis
    redisContext *redis_conn = redis_utils::RedisConnect("127.0.0.1", 6379);
    if (redis_conn == nullptr) {
        res.code = 500;
        res.write("Redis connection error");
        res.end();
        return;
    }

    // Save request to redis
    redis_utils::RedisSaveVideoRequest(redis_conn, video_request);

    // Create a RequestsChain and perform the first HTTP POST request
    asio::io_context io_context;
    utils::http::RequestsChain chain(io_context);

    crow::json::wvalue body;
    body["redis_id"] = id;
    body["video_path"] = video_path;
    chain.AddRequest("127.0.0.1", "8081", "/process_video", body, 
        [&chain, id](const crow::response& response) {
            OnProcessVideoComplete(response, chain, id);
        });
    chain.Execute();

    res.code = 200;
    res.write(id);
    res.end();
}

} // namespace

void BindSubmitVideoHandler(crow::SimpleApp& app) {
    CROW_ROUTE(app, "/submit_video").methods(crow::HTTPMethod::POST)(SubmitVideoHandler);
}

} // namespace handlers
