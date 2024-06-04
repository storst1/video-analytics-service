#include "process_video.h"

#include <iostream>
#include <cstdlib>
#include <filesystem>
#include <string>

#include "../../../../utils/redis/redis.h"
#include "../../../../utils/cfg/global_config.h"


#ifdef _WIN32
    #define popen _popen
    #define pclose _pclose
#endif


namespace handlers {

namespace fs = std::filesystem;

namespace {

/**
 * Returns the duration of a video file.
 * 
 * @param video_path The path to the video file.
 * @return The duration of the video in seconds, or -1 if an error occurred.
 */
int GetVideoDuration(const std::string& video_path) {
    std::string command = "ffprobe -v error -show_entries format=duration -of default=noprint_wrappers=1:nokey=1 " + video_path;
    FILE* pipe = popen(command.c_str(), "r");
    if (!pipe) {
        std::cerr << "Error: Failed to execute ffprobe command" << std::endl;
        return -1;
    }

    char buffer[128];
    std::string result = "";
    while (!feof(pipe)) {
        if (fgets(buffer, 128, pipe) != NULL) {
            result += buffer;
        }
    }
    pclose(pipe);

    try {
        double duration = std::stod(result);
        return static_cast<int>(duration) + 1;
    } catch (const std::exception& e) {
        std::cerr << "Error: Failed to parse video duration" << std::endl;
        return -1;
    }
}

/**
 * @brief Splits the frames in the specified directory into multiple subdirectories.
 * 
 * This function takes a directory path as input and splits the frames in that directory
 * into multiple subdirectories. Each subdirectory contains a specified number of frames.
 * The frames are moved from the original directory to the subdirectories based on the
 * specified number of frames per directory.
 * 
 * @param frames_path The path of the directory containing the frames.
 */
void SplitFramesIntoDirectories(const std::string& frames_path) {
    std::size_t frame_count = 0;
    std::size_t dir_count = 0;
    const int frames_per_directory = 60;
    std::string current_dir = frames_path + "../dir_" + std::to_string(dir_count);
    fs::create_directory(current_dir);

    for (const auto& entry : fs::directory_iterator(frames_path)) {
        if (entry.is_regular_file()) {
            std::string frame_path = entry.path().string();
            std::string new_frame_path = current_dir + "/" + entry.path().filename().string();
            fs::rename(frame_path, new_frame_path);
            frame_count++;

            if (frame_count >= frames_per_directory) {
                frame_count = 0;
                dir_count++;
                current_dir = frames_path + "/dir_" + std::to_string(dir_count);
                fs::create_directory(current_dir);
            }
        }
    }
}

/**
 * @brief Deletes all the original frames in the specified directory.
 * 
 * This function iterates over all the files in the given directory and deletes
 * any regular files found. It does not delete directories or other types of files.
 * 
 * @param frames_path The path to the directory containing the original frames.
 */
void DeleteOriginalFrames(const std::string& frames_path) {
    for (const auto& entry : fs::directory_iterator(frames_path)) {
        if (entry.is_regular_file()) {
            fs::remove(entry.path());
        }
    }
}

/**
 * Process the frames of a video.
 *
 * This function splits the frames into directories and deletes the original frames.
 *
 * @param frames_path The path to the directory containing the frames.
 */
void ProcessFrames(const std::string& frames_path) {
    SplitFramesIntoDirectories(frames_path);
    DeleteOriginalFrames(frames_path);
}

/**
 * Extracts frames from a video file and saves them as individual images.
 * 
 * @param video_path The path to the video file.
 * @param output_path The path to the directory where the extracted frames will be saved.
 */
bool ExtractFrames(const std::string& video_path, const std::string& output_path) {
    const std::string command = std::string(FFMPEG_EXECUTABLE) + " -i " + video_path + " -vf fps=1 " + output_path + "/frame_%04d.png";
    int result = std::system(command.c_str());
    if (result != 0) {
        std::cerr << "Error: FFmpeg command failed with code " << result << std::endl;
        return false;
    }
    return true;
}

/**
 * Resizes all images in a folder using ffmpeg.
 * 
 * @param folder_path The path to the folder containing the images.
 * @param width The width of the resized images.
 * @param height The height of the resized images.
 * @return true if the images were resized successfully, false otherwise.
 */
bool ResizeImagesInFolder(const std::string& folder_path, const std::size_t width, const std::size_t height) {
    fs::create_directory(folder_path + "/resized");
    const std::string command = "ffmpeg -y -hide_banner -loglevel panic -r 1 -f image2 -i \"" + 
                           folder_path + "/%*.png\" -vf scale=" + std::to_string(width) + ":" 
                           + std::to_string(height) + " \"" + folder_path + "/resized_%*.png\"";
    std::cout << "Resizing images using command: " << command << std::endl;
    int result = std::system(command.c_str());
    if (result != 0) {
        std::cerr << "Error: Failed to resize images using ffmpeg." << std::endl;
        return false;
    }
    return true;
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
            return crow::response(401, "Invalid JSON");
        }

        const std::string video_path = body["video_path"].s();
        const std::string redis_id = body["redis_id"].s();
        const std::string output_path = "../../../tmp/frames/frames-" + redis_id;
        
        const auto& config = cfg::GlobalConfig::getInstance();
        const auto& redis = config.getRedis();
        auto redis_conn = redis_utils::RedisConnect(redis.host, redis.port);

        const auto status_opt = redis_utils::RedisGetRequestVideoStatus(redis_conn, redis_id);
        if (!status_opt.has_value()) {
            return crow::response(500, "Failed to get video status from Redis");
        }
        const auto& status = status_opt.value();
        if (status == requests::VideoStatus::Failed || status == requests::VideoStatus::Stopped) {
            return crow::response(400, "Pipe broken by video status = " + requests::VideoStatusToString(status));
        }

        // const int video_duration = GetVideoDuration(video_path);
        // if (video_duration == -1) {
        //    return crow::response(500, "Failed to get video duration");
        // }

        // Create initial dir for frames
        fs::create_directory(output_path);

        const bool extraction_success = ExtractFrames(video_path, output_path);
        if (!extraction_success) {
            return crow::response(500, "Failed to extract frames from video");
        }

        // Resize images
        constexpr std::size_t resize_width = 600;
        constexpr std::size_t resize_height = 400;
        const bool resize_success = ResizeImagesInFolder(output_path, resize_width, resize_height);
        if (!resize_success) {
            return crow::response(500, "Failed to resize images");
        }

        if (status == requests::VideoStatus::Failed || status == requests::VideoStatus::Stopped) {
            return crow::response(400, "Pipe broken by video status = " + requests::VideoStatusToString(status));
        }

        // Process the frames
        ProcessFrames(output_path + "/resized");

        return crow::response(200, "Processing finished.");
    });
}

} // namespace handlers
