#pragma once

#include <functional>
#include <string>
#include <deque>

#include <asio.hpp>
#include <crow.h>

namespace utils::http {

class RequestChain {
public:
    using Callback = std::function<void(bool)>;

    RequestChain(asio::io_context& io_context);

    void AddRequest(const std::string& host, const std::string& port, const std::string& target, const crow::json::wvalue& body, Callback callback);
    void Execute();

private:
    struct Request {
        std::string host;
        std::string port;
        std::string target;
        crow::json::wvalue body;
        Callback callback;

        Request(const std::string& h, const std::string& p, const std::string& t, const crow::json::wvalue& b, Callback c);
    };

    void ExecuteNext();

    asio::io_context& io_context_;
    asio::ip::tcp::socket socket_;
    std::deque<Request> requests_;
};

} //namespace utils::http