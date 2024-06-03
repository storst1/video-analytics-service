#include "status.h"

#include "../../utils/redis/redis.h"
#include "../../utils/cfg/global_config.h"

namespace handlers {

/**
 * Binds the status handler to the given Crow application.
 *
 * @param app The Crow application to bind the status handler to.
 */
void BindStatusHandler(crow::SimpleApp& app) {
    CROW_ROUTE(app, "/status/<string>").methods(crow::HTTPMethod::GET)
    ([](const crow::request& req, std::string id){
        const auto& config = cfg::GlobalConfig::getInstance();
        const auto& redis = config.getRedis();
        redisContext *redis_conn = redis_utils::RedisConnect(redis.host, redis.port);
        if (redis_conn == nullptr) {
            return crow::response(500, "Redis connection error");
        }

        const auto i = id.find("request:");
        if (i != std::string::npos) {
            id = id.substr(i + 8); // 8 is the length of "request:"
        }

        redisReply *reply = redis_utils::RedisGetByKey(redis_conn, "HGETALL request:%s", id.c_str());
        if (reply == nullptr) {
            return crow::response(404, "Video not found");
        }

        crow::json::wvalue response;
        for (size_t i = 0; i < reply->elements; i += 2) {
            std::string key = reply->element[i]->str;
            std::string value = reply->element[i+1]->str;
            if (key == "id") {
                response["id"] = value;
            }
            else if (key == "path") { 
                response["path"] = value;
            }
            else if (key == "status") { 
                response["status"] = value;
            }
        }

        freeReplyObject(reply);
        redisFree(redis_conn);

        return crow::response(200, response);
    });
}

} // namespace handlers