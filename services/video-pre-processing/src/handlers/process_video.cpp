#include "process_video.h"

#include <iostream>
#include <cstdlib>
#include <filesystem>
#include <string>

namespace handlers {

namespace fs = std::filesystem;

namespace {

/**
 * Extracts frames from a video file and saves them as individual images.
 * 
 * @param video_path The path to the video file.
 * @param output_path The path to the directory where the extracted frames will be saved.
 */
void ExtractFrames(const std::string& video_path, const std::string& output_path) {
    std::string command = std::string(FFMPEG_EXECUTABLE) + " -i " + video_path + " -vf fps=1 " + output_path + "/frame_%04d.png";
    int result = std::system(command.c_str());
    if (result != 0) {
        std::cerr << "Error: FFmpeg command failed with code " << result << std::endl;
    }
}

} // namespace

/**
 * Binds the process_video handler to the specified Crow application.
 * This handler processes a video by extracting frames from it and saving them to a directory.
 * 
 * @param app The Crow application to bind the handler to.
 */
void BindProcessVideoHandler(crow::SimpleApp& app) {
    CROW_ROUTE(app, "/process_video").methods(crow::HTTPMethod::POST)
    ([](const crow::request& req) {
        auto body = crow::json::load(req.body);
        if (!body) {
            return crow::response(400, "Invalid JSON");
        }

        std::string video_path = body["video_path"].s();
        std::string redis_id = body["redis_id"].s();
        std::string output_path = "../../../tmp/frames/frames-" + redis_id;

        // Создаем директорию для фреймов
        fs::create_directory(output_path);

        ExtractFrames(video_path, output_path);
        return crow::response(200, "Processing started.");
    });
}

} // namespace handlers
