#include <iostream>

#include <crow.h>
#include <hiredis.h>

#ifdef _WIN32
#include <objbase.h>
#pragma comment(lib, "ole32.lib")
#else
#include <uuid/uuid.h>
#endif

struct VideoRequest {
    std::string id;
    std::string path;
    std::string status;
};

std::string generate_uuid() {
    #ifdef _WIN32
    GUID guid;
    CoCreateGuid(&guid);

    // Buffer to hold the UUID string
    char uuid_str[37];
    // Format the GUID into a UUID string
    snprintf(uuid_str, sizeof(uuid_str),
             "%08x-%04x-%04x-%04x-%012llx",
             (unsigned int)guid.Data1, guid.Data2, guid.Data3,
             (guid.Data4[0] << 8) | guid.Data4[1],
             (unsigned long long)((guid.Data4[2] << 40) | (guid.Data4[3] << 32) |
                                  (guid.Data4[4] << 24) | (guid.Data4[5] << 16) |
                                  (guid.Data4[6] << 8) | guid.Data4[7]));
    // Return the UUID string
    return std::string(uuid_str);
    #else
    uuid_t uuid;
    uuid_generate(uuid);
    char uuid_str[37];
    // Convert the UUID to a string
    uuid_unparse(uuid, uuid_str);
    return std::string(uuid_str);
    #endif
}

int main() {
    crow::SimpleApp app;

    CROW_ROUTE(app, "/submit_video").methods(crow::HTTPMethod::POST)
    ([](const crow::request& req){
        auto video_path = req.body;
        std::string id = generate_uuid();
        VideoRequest request = {id, video_path, "received"};

        // Подключение к Redis
        redisContext *c = redisConnect("127.0.0.1", 6379);
        if (c == NULL || c->err) {
            if (c) {
                std::cerr << "Error: " << c->errstr << std::endl;
                redisFree(c);
            } else {
                std::cerr << "Can't allocate redis context" << std::endl;
            }
            return crow::response(500, "Redis connection error");
        }

        // Сохранение запроса в Redis
        redisCommand(c, "HMSET request:%s id %s path %s status %s", 
                     id.c_str(), request.id.c_str(), request.path.c_str(), request.status.c_str());

        redisFree(c);

        return crow::response(200, id);
    });

    CROW_ROUTE(app, "/status/<string>")
    ([](const crow::request& req, std::string id){
        // Подключение к Redis
        redisContext *c = redisConnect("127.0.0.1", 6379);
        if (c == NULL || c->err) {
            if (c) {
                std::cerr << "Error: " << c->errstr << std::endl;
                redisFree(c);
            } else {
                std::cerr << "Can't allocate redis context" << std::endl;
            }
            return crow::response(500, "Redis connection error");
        }

        // Получение данных запроса из Redis
        redisReply *reply = (redisReply *)redisCommand(c, "HGETALL request:%s", id.c_str());
        if (reply == NULL || reply->type == REDIS_REPLY_NIL) {
            if (reply) freeReplyObject(reply);
            redisFree(c);
            return crow::response(404, "Request not found");
        }

        VideoRequest request;
        for (size_t i = 0; i < reply->elements; i += 2) {
            std::string key = reply->element[i]->str;
            std::string value = reply->element[i+1]->str;
            if (key == "id") request.id = value;
            else if (key == "path") request.path = value;
            else if (key == "status") request.status = value;
        }

        freeReplyObject(reply);
        redisFree(c);

        crow::json::wvalue response;
        response["id"] = request.id;
        response["path"] = request.path;
        response["status"] = request.status;

        return crow::response(200, response);
    });

    app.port(8080).multithreaded().run();

    return 0;
}
