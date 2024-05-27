#ifndef REQUESTS_CHAIN_H
#define REQUESTS_CHAIN_H

#include <asio.hpp>
#include <crow.h>
#include <vector>
#include <functional>

namespace utils {
namespace http {

class RequestsChain {
public:
    using ResponseHandler = std::function<void(const crow::response&)>;

    RequestsChain(asio::io_context& io_context)
        : io_context_(io_context) {}

    void AddRequest(const std::string& host, const std::string& port, const std::string& target, 
                    const crow::json::wvalue& body, ResponseHandler handler);

    void Execute();

private:
    asio::io_context& io_context_;
    std::vector<std::tuple<std::string, std::string, std::string, crow::json::wvalue, ResponseHandler>> requests_;
};

} // namespace http
} // namespace utils

#endif // REQUESTS_CHAIN_H
