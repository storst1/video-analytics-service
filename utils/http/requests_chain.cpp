#include "requests_chain.h"

#include <iostream>

namespace utils {
namespace http {

void RequestsChain::AddRequest(const std::string& host, const std::string& port, const std::string& target, 
                               const crow::json::wvalue& body, ResponseHandler handler) {
    requests_.emplace_back(host, port, target, body, handler);
}

void RequestsChain::Execute() {
    if (requests_.empty()) return;

    auto [host, port, target, body, handler] = requests_.front();
    requests_.erase(requests_.begin());

    try {
        asio::ip::tcp::resolver resolver(io_context_);
        asio::ip::tcp::resolver::results_type endpoints = resolver.resolve(host, port);

        asio::ip::tcp::socket socket(io_context_);
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

        crow::response crow_response;
        crow_response.code = status_code;

        if (response.size() > 0) {
            std::ostringstream ss;
            ss << &response;
            crow_response.body = ss.str();
        }

        handler(crow_response);

        asio::error_code ec;
        while (asio::read(socket, response, asio::transfer_at_least(1), ec)) {
            std::ostringstream ss;
            ss << &response;
            crow_response.body += ss.str();
        }

        if (ec != asio::error::eof) {
            throw asio::system_error(ec);
        }

        // Execute the next request in the chain
        Execute();

    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
    }
}

} // namespace http
} // namespace utils
