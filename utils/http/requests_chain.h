#ifndef REQUESTS_CHAIN_H
#define REQUESTS_CHAIN_H

#include <asio.hpp>
#include <crow.h>

#include <vector>
#include <functional>
#include <string>

namespace utils {
namespace http {

/**
 * @brief Represents a chain of HTTP requests.
 * 
 * The RequestsChain class allows you to add multiple HTTP requests to a chain and execute them sequentially.
 * Each request in the chain can have its own host, port, target, body, and response handler.
 */
class RequestsChain {
public:
    using ResponseHandler = std::function<void(const crow::response&)>;

    /**
     * @brief Constructs a RequestsChain object.
     * 
     * @param io_context The asio::io_context object to be used for asynchronous operations.
     */
    RequestsChain(asio::io_context& io_context)
        : io_context_(io_context) {}

    /**
     * @brief Adds a request to the chain.
     * 
     * @param host The host of the request.
     * @param port The port of the request.
     * @param target The target of the request.
     * @param body The body of the request.
     * @param handler The response handler for the request.
     */
    void AddRequest(const std::string& host, const std::string& port, const std::string& target, 
                    const crow::json::wvalue& body, ResponseHandler handler);

    /**
     * @brief Executes the requests in the chain.
     * 
     * The requests are executed sequentially in the order they were added to the chain.
     * The response handlers are called for each request after the response is received.
     * 
     * @return true if all requests were executed successfully, false otherwise.
     */
    bool Execute();

    std::size_t GetRequestsCount() const {
        return requests_.size();
    }

    bool IsEmpty() const {
        return requests_.empty();
    }

private:
    asio::io_context& io_context_;
    std::vector<std::tuple<std::string, std::string, std::string, crow::json::wvalue, ResponseHandler>> requests_;
};

} // namespace http
} // namespace utils

#endif // REQUESTS_CHAIN_H
