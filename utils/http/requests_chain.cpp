#include "requests_chain.h"

#include <iostream>

namespace utils::http {

RequestChain::RequestChain(asio::io_context& io_context)
    : io_context_(io_context), socket_(io_context) {}

void RequestChain::AddRequest(const std::string& host, const std::string& port, const std::string& target, const crow::json::wvalue& body, Callback callback) {
    requests_.emplace_back(host, port, target, body, callback);
}

void RequestChain::Execute() {
    if (!requests_.empty()) {
        ExecuteNext();
    }
}

RequestChain::Request::Request(const std::string& h, const std::string& p, const std::string& t, const crow::json::wvalue& b, Callback c)
    : host(h), port(p), target(t), body(b), callback(c) {}

void RequestChain::ExecuteNext() {
    if (requests_.empty()) {
        return;
    }

    auto& req = requests_.front();

    try {
        asio::ip::tcp::resolver resolver(io_context_);
        asio::ip::tcp::resolver::results_type endpoints = resolver.resolve(req.host, req.port);

        asio::connect(socket_, endpoints);

        const std::string body_str = req.body.dump();

        asio::streambuf request;
        std::ostream request_stream(&request);
        request_stream << "POST " << req.target << " HTTP/1.1\r\n";
        request_stream << "Host: " << req.host << "\r\n";
        request_stream << "Content-Type: application/json\r\n";
        request_stream << "Content-Length: " << body_str.length() << "\r\n";
        request_stream << "Connection: close\r\n\r\n";
        request_stream << body_str;

        asio::write(socket_, request);

        asio::streambuf response;
        asio::read_until(socket_, response, "\r\n");

        std::istream response_stream(&response);
        std::string http_version;
        response_stream >> http_version;
        unsigned int status_code;
        response_stream >> status_code;
        std::string status_message;
        std::getline(response_stream, status_message);

        if (!response_stream || http_version.substr(0, 5) != "HTTP/") {
            std::cout << "Invalid response\n";
            req.callback(false);
            return;
        }

        if (status_code != 200) {
            std::cout << "Response returned with status code " << status_code << "\n";
            req.callback(false);
            return;
        }

        asio::read_until(socket_, response, "\r\n\r\n");

        std::string header;
        while (std::getline(response_stream, header) && header != "\r") {
            std::cout << header << "\n";
        }

        if (response.size() > 0) {
            std::cout << &response;
        }

        asio::error_code ec;
        while (asio::read(socket_, response, asio::transfer_at_least(1), ec)) {
            std::cout << &response;
        }

        if (ec != asio::error::eof) {
            throw asio::system_error(ec);
        }

        req.callback(true);
    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
        req.callback(false);
    }

    requests_.pop_front();
    if (!requests_.empty()) {
        ExecuteNext();
    }
}

} // namespace utils::http