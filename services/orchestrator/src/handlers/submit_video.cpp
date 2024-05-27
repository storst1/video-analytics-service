#include "submit_video.h"
#include "../../utils/http/requests_chain.h"
#include <iostream>
#include <asio.hpp>
#include "../../utils/redis/redis.h"

namespace handlers {

namespace {

void OnYoloAnalyzeComplete(bool success) {
    if (success) {
        std::cout << "YOLO analysis started successfully\n";
    } else {
        std::cout << "Failed to start YOLO analysis\n";
    }
}

void OnProcessVideoComplete(bool success, utils::http::RequestChain& chain, const std::string& id) {
    if (success) {
        crow::json::wvalue yolo_body;
        yolo_body["redis_id"] = id;
        std::string frames_folder = "./frames-" + id;
        yolo_body["frames_path"] = frames_folder;
        chain.AddRequest("127.0.0.1", "8081", "/yolo_analyze_frames", yolo_body, OnYoloAnalyzeComplete);
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

    // Create a RequestChain and perform the first HTTP POST request
    asio::io_context io_context;
    utils::http::RequestChain chain(io_context);

    crow::json::wvalue body;
    body["redis_id"] = id;
    body["video_path"] = video_path;
    chain.AddRequest("127.0.0.1", "8081", "/process_video", body, 
        std::bind(OnProcessVideoComplete, std::placeholders::_1, std::ref(chain), id));
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
