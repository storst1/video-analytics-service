#include "stop.h"

#include "../../utils/cfg/global_config.h"
#include "../../utils/redis/redis.h"
#include "../../utils/db/pg.h"

namespace handlers {

void BindStopHandler(crow::SimpleApp& app) {
    CROW_ROUTE(app, "/stop").methods(crow::HTTPMethod::POST)
    ([](const crow::request& req) {
        const auto& id = req.body;

        const auto& config = cfg::GlobalConfig::getInstance();
        const auto& redis_config = config.getRedis();
        auto redis_conn = redis_utils::RedisConnect(redis_config.host, redis_config.port);
        if (redis_conn == nullptr) {
            return crow::response(500, "Failed to connect to Redis");
        }

        const auto redis_status = redis_utils::RedisGetByKey(redis_conn, "HGETALL request:%s", id.c_str());
        if (redis_status == nullptr) {
            return crow::response(500, "No such key in Redis");
        }

        // Set status to Stopped
        redis_utils::RedisUpdateVideoStatus(redis_conn, id, requests::VideoStatus::Stopped);
        utils::db::UpdateVideoStatus(id, requests::VideoStatusToString(requests::VideoStatus::Stopped));

        return crow::response(200, "Video stopped");
    });
}

} // namespace handlers