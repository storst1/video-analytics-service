#include "submit_video.h"

#include <iostream>

#include <asio.hpp>

#include "../redis/redis.h"

namespace handlers {

namespace {

void PerformHttpPost(const std::string& host, const std::string& port, 
                     const std::string& target, const crow::json::wvalue& body) {
    try {
        asio::io_context io_context;
        asio::ip::tcp::resolver resolver(io_context);
        asio::ip::tcp::resolver::results_type endpoints = resolver.resolve(host, port);

        asio::ip::tcp::socket socket(io_context);
        asio::connect(socket, endpoints);

        const std::string body_str = body.dump();

        asio::streambuf request;
        std::ostream request_stream(&request);
        request_stream << "POST " << target << " HTTP/1.1\r\n";
        request_stream << "Host: " << host << "\r\n";
        request_stream << "Content-Type: application/x-www-form-urlencoded\r\n";
        request_stream << "Content-Length: " << body_str.length() << "\r\n";
        request_stream << "Connection: close\r\n\r\n";
        request_stream << body_str;

        asio::write(socket, request);

        asio::streambuf response;
        asio::read_until(socket, response, "\r\n");

        std::istream response_stream(&response);
        std::string http_version;
        response_stream >> http_version;
        unsigned int status_code;
        response_stream >> status_code;
        std::string status_message;
        std::getline(response_stream, status_message);

        if (!response_stream || http_version.substr(0, 5) != "HTTP/") {
            std::cout << "Invalid response\n";
            return;
        }

        if (status_code != 200) {
            std::cout << "Response returned with status code " << status_code << "\n";
            return;
        }

        asio::read_until(socket, response, "\r\n\r\n");

        std::string header;
        while (std::getline(response_stream, header) && header != "\r") {
            std::cout << header << "\n";
        }

        if (response.size() > 0) {
            std::cout << &response;
        }

        asio::error_code ec;
        while (asio::read(socket, response, asio::transfer_at_least(1), ec)) {
            std::cout << &response;
        }

        if (ec != asio::error::eof) {
            throw asio::system_error(ec);
        }
    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
    }
}

} // namespace

void BindSubmitVideoHandler(crow::SimpleApp& app) {
    CROW_ROUTE(app, "/submit_video").methods(crow::HTTPMethod::POST)
    ([](const crow::request& req){
        auto video_path = req.body;
        std::string id = redis_utils::GenerateUUID();
        requests::VideoRequest video_request = {id, video_path, requests::VideoStatus::Received};

        // Connect to redis
        redisContext *redis_conn = redis_utils::RedisConnect("127.0.0.1", 6379);
        if (redis_conn == nullptr) {
            return crow::response(500, "Redis connection error");
        }

        // Save request to redis
        redis_utils::RedisSaveVideoRequest(redis_conn, video_request);

        // Perform HTTP POST request to the video pre-processing service
        crow::json::wvalue body;
        body["redis_id"] = id;
        body["video_path"] = video_path;
        PerformHttpPost("127.0.0.1", "8081", "/process_video", body);

        return crow::response(200, id);
    });
}

} // namespace handlers
