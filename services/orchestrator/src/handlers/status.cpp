#include "status.h"

#include "../redis/redis.h"

namespace handlers {

void BindStatusHandler(crow::SimpleApp& app) {
    CROW_ROUTE(app, "/status/<string>")
    ([](const crow::request& req, std::string id){
        redisContext *redis_conn = redis_utils::RedisConnect("127.0.0.1", 6379);
        if (redis_conn == nullptr) {
            return crow::response(500, "Redis connection error");
        }

        redisReply *reply = redis_utils::RedisGetByKey(redis_conn, "HGETALL request:%s", id.c_str());
        if (reply == nullptr) {
            return crow::response(404, "Request not found");
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