#include "submit_video.h"

#include <iostream>

#include <asio.hpp>

#include "../../utils/http/requests_chain.h"
#include "../../utils/http/requests.h"
#include "../../utils/redis/redis.h"
#include "../../utils/cfg/global_config.h"
#include "../../utils/db/pg.h"

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
        const auto& redis = config.getRedis();
        redisContext *redis_conn = redis_utils::RedisConnect(redis.host, redis.port);
        if (redis_conn == nullptr) {
            return;
        }
        redis_utils::RedisUpdateVideoStatus(redis_conn, id, requests::VideoStatus::YoloFinished);
        chain.AddRequest(video_post.host, std::to_string(video_post.port), "/save_video", save_body, 
[&redis_conn, &id](const crow::response& res) {
            if (res.code == 200) {
                std::cout << "Video analysis saved successfully\n";
                redis_utils::RedisUpdateVideoStatus(redis_conn, id, requests::VideoStatus::Finished);
                utils::db::UpdateVideoStatus(id, requests::VideoStatusToString(requests::VideoStatus::Finished));
            } else {
                std::cout << "Failed to save video analysis\n";
                redis_utils::RedisUpdateVideoStatus(redis_conn, id, requests::VideoStatus::Failed);
                utils::db::UpdateVideoStatus(id, requests::VideoStatusToString(requests::VideoStatus::Failed));
            }
        });
        redis_utils::RedisUpdateVideoStatus(redis_conn, id, requests::VideoStatus::PostProcessing);
        const bool able_to_exec = chain.Execute();
        if (!able_to_exec){
            redis_utils::RedisUpdateVideoStatus(redis_conn, id, requests::VideoStatus::Failed);
            utils::db::UpdateVideoStatus(id, requests::VideoStatusToString(requests::VideoStatus::Failed));
        }
    } else {
        std::cout << "Failed to start or finish YOLO analysis\n" << ". Response.body: " << response.body << std::endl;
        const auto& config = cfg::GlobalConfig::getInstance();
        const auto& redis = config.getRedis();
        redisContext *redis_conn = redis_utils::RedisConnect(redis.host, redis.port);
        if (redis_conn == nullptr) {
            return;
        }
        redis_utils::RedisUpdateVideoStatus(redis_conn, id, requests::VideoStatus::Failed);
        utils::db::UpdateVideoStatus(id, requests::VideoStatusToString(requests::VideoStatus::Failed));
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
    const auto& config = cfg::GlobalConfig::getInstance();
    const auto& redis = config.getRedis();
    redisContext *redis_conn = redis_utils::RedisConnect(redis.host, redis.port);
    if (redis_conn == nullptr) {
        return;
    }
    if (response.code == 200) {
        redis_utils::RedisUpdateVideoStatus(redis_conn, id, requests::VideoStatus::PreProcessingFinished);

        crow::json::wvalue yolo_body;
        yolo_body["redis_id"] = id;
        std::string frames_folder = std::filesystem::absolute("../../../tmp/frames/frames-" + id).string();
        yolo_body["frames_path"] = frames_folder;
        const auto& frame_analytics = config.getFrameAnalytics();
        chain.AddRequest(frame_analytics.host, std::to_string(frame_analytics.port), "/yolo_analyze_frames", yolo_body,
            std::bind(OnYoloAnalyzeComplete, std::placeholders::_1, std::ref(chain), id));

        redis_utils::RedisUpdateVideoStatus(redis_conn, id, requests::VideoStatus::YoloStarted);
        const bool able_to_exec = chain.Execute();
        if (!able_to_exec) {
            redis_utils::RedisUpdateVideoStatus(redis_conn, id, requests::VideoStatus::Failed);
            utils::db::UpdateVideoStatus(id, requests::VideoStatusToString(requests::VideoStatus::Failed));
        }
    } else {
        std::cout << "Failed to start video processing\n";
        redis_utils::RedisUpdateVideoStatus(redis_conn, id, requests::VideoStatus::Failed);
        utils::db::UpdateVideoStatus(id, requests::VideoStatusToString(requests::VideoStatus::Failed));
    }
    redisFree(redis_conn);
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

    // Save video to database
    utils::db::SaveRequestOnReceive(video_request.id);

    // Create a RequestsChain and perform the first HTTP POST request
    asio::io_context io_context;
    auto chain = std::make_shared<utils::http::RequestsChain>(io_context); // Используем shared_ptr для цепочки запросов

    crow::json::wvalue body;
    body["redis_id"] = id;
    body["video_path"] = video_path;

    const auto& pre_processing = config.getVideoPreProcessing();
    chain->AddRequest(pre_processing.host, std::to_string(pre_processing.port), "/process_video", body,
    [chain, id](const crow::response& response) { // Используем shared_ptr для цепочки запросов
        OnProcessVideoComplete(response, *chain, id);
    });

    redis_utils::RedisUpdateVideoStatus(redis_conn, id, requests::VideoStatus::PreProcessingStarted);
    
    io_context.post([chain] {
        chain->Execute();
    });

    res.code = 200;
    res.write(id);
    res.end();
}

} // namespace

void BindSubmitVideoHandler(crow::SimpleApp& app) {
    CROW_ROUTE(app, "/submit_video").methods(crow::HTTPMethod::POST)(SubmitVideoHandler);
}

} // namespace handlers
