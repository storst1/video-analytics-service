#include "process_video.h"

#include <iostream>
#include <cstdlib>
#include <filesystem>
#include <string>

namespace handlers {

namespace fs = std::filesystem;

namespace {

void ExtractFrames(const std::string& video_path, const std::string& output_path) {
    std::string command = std::string(FFMPEG_EXECUTABLE) + " -i " + video_path + " -vf fps=1 " + output_path + "/frame_%04d.png";
    int result = std::system(command.c_str());
    if (result != 0) {
        std::cerr << "Error: FFmpeg command failed with code " << result << std::endl;
    }
}

} // namespace

void BindProcessVideoHandler(crow::SimpleApp& app) {
    CROW_ROUTE(app, "/process_video").methods(crow::HTTPMethod::POST)
    ([](const crow::request& req) {
        auto body = crow::json::load(req.body);
        if (!body) {
            return crow::response(400, "Invalid JSON");
        }

        std::string video_path = body["video_path"].s();
        std::string redis_id = body["redis_id"].s();
        std::string output_path = "./frames-" + redis_id;

        // Создаем директорию для фреймов
        fs::create_directory(output_path);

        ExtractFrames(video_path, output_path);
        return crow::response(200, "Processing started.");
    });
}

} // namespace handlers
