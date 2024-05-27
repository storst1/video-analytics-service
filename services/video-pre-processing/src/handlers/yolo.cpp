#include "yolo.h"

#include <iostream>
#include <cstdlib>
#include <array>
#include <cstdio>
#include <memory>
#include <thread>
#include <future>
#include <crow.h>

namespace handlers {

namespace {

std::string RunYoloScript(const std::string& folder_path) {
    std::string command = "python3 ../yolo/yolo_analyze.py " + folder_path;
    std::array<char, 128> buffer;
    std::string result;
    std::shared_ptr<FILE> pipe(_popen(command.c_str(), "r"), _pclose);
    if (!pipe) throw std::runtime_error("popen() failed!");
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }
    return result;
}

std::future<std::string> RunYoloScriptAsync(const std::string& folder_path) {
    std::cout << "Running yolo script with folder_path=" << folder_path << std::endl;
    return std::async(std::launch::async, RunYoloScript, folder_path);
}

} // namespace

void BindYoloHandler(crow::SimpleApp& app) {
    CROW_ROUTE(app, "/yolo_analyze_frames").methods(crow::HTTPMethod::POST)
    ([](const crow::request& req, crow::response& res) {
        auto folder_path = req.body;
        try {
            auto result_future = RunYoloScriptAsync(folder_path);
            result_future.wait();
            std::string result = result_future.get();
            std::cout << result << std::endl;
            res.write(result);
            res.end();
        } catch (const std::exception& e) {
            res.code = 500;
            res.write(e.what());
            res.end();
        }
    });
}

} // namespace handlers
