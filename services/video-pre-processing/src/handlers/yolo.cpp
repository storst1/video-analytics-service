#include "yolo.h"

#include <iostream>
#include <cstdlib>
#include <array>
#include <cstdio>
#include <memory>

#ifdef _WIN32
#define popen _popen
#define pclose _pclose
#endif

namespace handlers {

namespace {

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

void BindYoloHandler(crow::SimpleApp& app) {
    CROW_ROUTE(app, "/yolo_analyze_frames").methods(crow::HTTPMethod::POST)
    ([](const crow::request& req) {
        auto folder_path = req.body;
        try {
            std::string result = RunYoloScript(folder_path);
            return crow::response(200, result);
        } catch (const std::exception& e) {
            return crow::response(500, e.what());
        }
    });
}

} // namespace handlers