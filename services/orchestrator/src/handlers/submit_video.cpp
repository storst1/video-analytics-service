#include "submit_video.h"

#include <iostream>

#include <asio.hpp>

#include "../../utils/http/requests_chain.h"
#include "../../utils/redis/redis.h"
#include "../../utils/cfg/global_config.h"

namespace handlers {

namespace {

/**
 * Callback function called when YOLO analysis is complete.
 * 
 * @param response The response from the YOLO analysis request.
 * @param chain The HTTP requests chain.
 * @param id The ID of the video analysis.
 */
void OnYoloAnalyzeComplete(const crow::response& response, utils::http::RequestsChain& chain, const std::string& id) {
    if (response.code == 200) {
        crow::json::wvalue save_body;
        save_body["redis_id"] = id;
        const auto& config = cfg::GlobalConfig::getInstance();
        const auto& video_post = config.getVideoPostProcessing();
        chain.AddRequest(video_post.host, std::to_string(video_post.port), "/save_video", save_body, 
[](const crow::response& res) {
            if (res.code == 200) {
                std::cout << "Video analysis saved successfully\n";
            } else {
                std::cout << "Failed to save video analysis\n";
            }
        });
        chain.Execute();
    } else {
        std::cout << "Failed to start or finish YOLO analysis\n" << ". Response.body: " << response.body << std::endl;
        const auto& config = cfg::GlobalConfig::getInstance();
        const auto& redis = config.getRedis();
        redisContext *redis_conn = redis_utils::RedisConnect(redis.host, redis.port);
        if (redis_conn == nullptr) {
            return;
        }
        redis_utils::RedisUpdateVideoStatus(redis_conn, id, requests::VideoStatus::Failed);
    }
}

/**
 * Callback function called when video processing is complete.
 * 
 * @param response The response object containing the result of the video processing.
 * @param chain The HTTP requests chain object.
 * @param id The ID of the video being processed.
 */
void OnProcessVideoComplete(const crow::response& response, utils::http::RequestsChain& chain, const std::string& id) {
    if (response.code == 200) {
        crow::json::wvalue yolo_body;
        yolo_body["redis_id"] = id;
        std::string frames_folder = std::filesystem::absolute("../../../tmp/frames/frames-" + id).string();
        yolo_body["frames_path"] = frames_folder;
        const auto& config = cfg::GlobalConfig::getInstance();
        const auto& frame_analytics = config.getFrameAnalytics();
        chain.AddRequest(frame_analytics.host, std::to_string(frame_analytics.port), "/yolo_analyze_frames", yolo_body,
            std::bind(OnYoloAnalyzeComplete, std::placeholders::_1, std::ref(chain), id));
        chain.Execute();
    } else {
        std::cout << "Failed to start video processing\n";
        const auto& config = cfg::GlobalConfig::getInstance();
        const auto& redis = config.getRedis();
        redisContext *redis_conn = redis_utils::RedisConnect(redis.host, redis.port);
        if (redis_conn == nullptr) {
            return;
        }
        redis_utils::RedisUpdateVideoStatus(redis_conn, id, requests::VideoStatus::Failed);
    }
}

/**
 * Handles the HTTP request for submitting a video.
 * 
 * @param req The HTTP request object.
 * @param res The HTTP response object.
 */
void SubmitVideoHandler(const crow::request& req, crow::response& res) {
    auto video_path = req.body;
    std::string id = redis_utils::GenerateUUID();
    requests::VideoRequest video_request = {id, video_path, requests::VideoStatus::Received};

    // Connect to redis
    const auto& config = cfg::GlobalConfig::getInstance();
    const auto& redis = config.getRedis();
    redisContext *redis_conn = redis_utils::RedisConnect(redis.host, redis.port);
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

    const auto& pre_processing = config.getVideoPreProcessing();
    chain.AddRequest(pre_processing.host, std::to_string(pre_processing.port), "/process_video", body,
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
