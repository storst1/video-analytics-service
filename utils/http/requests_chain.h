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
 * The RequestsChain class allows you to create a chain of HTTP requests that will be executed sequentially.
 * Each request in the chain can have its own host, port, target, body, and response handler.
 * The requests are executed in the order they were added to the chain.
 * 
 * Usage:
 * 1. Create an instance of RequestsChain, passing an asio::io_context object to the constructor.
 * 2. Add requests to the chain using the AddRequest() method.
 * 3. Call the Execute() method to start executing the requests in the chain.
 * 
 * Example:
 * @code
 * asio::io_context io_context;
 * utils::http::RequestsChain chain(io_context);
 * 
 * chain.AddRequest("example.com", "80", "/api/endpoint1", {{"param1", "value1"}}, [](const crow::response& response) {
 *     // Handle response for the first request
 * });
 * 
 * chain.AddRequest("example.com", "80", "/api/endpoint2", {{"param2", "value2"}}, [](const crow::response& response) {
 *     // Handle response for the second request
 * });
 * 
 * chain.Execute();
 * io_context.run();
 * @endcode
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
     */
    void Execute();

private:
    asio::io_context& io_context_;
    std::vector<std::tuple<std::string, std::string, std::string, crow::json::wvalue, ResponseHandler>> requests_;
};

} // namespace http
} // namespace utils

#endif // REQUESTS_CHAIN_H
