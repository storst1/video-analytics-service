#include "yolo.h"
#include <iostream>
#include <cstdlib>
#include <array>
#include <cstdio>
#include <memory>
#include <filesystem>
#include "../../../../utils/redis/redis.h"

#ifdef _WIN32
    #define popen _popen
    #define pclose _pclose
#endif

namespace handlers {

namespace {

/**
 * Runs the YOLO script to analyze the frames in the specified folder.
 * 
 * @param folder_path The path to the folder containing the frames.
 * @return A string containing the result of the YOLO script execution.
 * @throws std::runtime_error if the popen() function fails.
 */
std::string RunYoloScript(const std::string& folder_path) {
    std::string command = "python3 ../yolo/yolo_analyze.py " + folder_path;
    std::array<char, 128> buffer;
    std::string result;
    std::shared_ptr<FILE> pipe(popen(command.c_str(), "r"), pclose);
    if (!pipe) {
        throw std::runtime_error("popen() failed!");
    }
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }
    return result;
}

} // namespace

/**
 * Binds the YOLO handler to the specified Crow application.
 * The YOLO handler analyzes frames using the YOLO algorithm and saves the result to Redis.
 * 
 * @param app The Crow application to bind the handler to.
 */
void BindYoloHandler(crow::SimpleApp& app) {
    CROW_ROUTE(app, "/yolo_analyze_frames").methods(crow::HTTPMethod::POST)
    ([](const crow::request& req) {
        auto body = crow::json::load(req.body);
        if (!body) {
            return crow::response(400, "Invalid JSON");
        }

        std::string redis_id = body["redis_id"].s();
        std::string frames_path = body["frames_path"].s();

        try {
            std::string result_str = RunYoloScript(frames_path);
            auto result_json = crow::json::load(result_str);

            // Connect to redis
            redisContext *redis_conn = redis_utils::RedisConnect("127.0.0.1", 6379);
            if (redis_conn == nullptr) {
                return crow::response(500, "Redis connection error");
            }

            std::cout << "Finished YOLO analysis" << std::endl;

            // Save YOLO result to redis
            redis_utils::RedisSaveYoloResponse(redis_conn, redis_id, result_json);
            redisFree(redis_conn);

            return crow::response(200, result_str);
        } catch (const std::exception& e) {
            return crow::response(500, e.what());
        }
    });
}

} // namespace handlers
