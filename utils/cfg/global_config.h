#pragma once

#include <string>

namespace cfg {

constexpr const char* CONFIG_FILE = "../../../config/config.json";

class GlobalConfig {
public:
    struct ServiceData {
        std::string host;
        std::size_t port;
    };

    struct DatabaseConfig {
        std::string dbname;
        std::string user;
        std::string password;
        std::string hostaddr;
        std::size_t port;

        std::string getConnectionString() const;
    };

    static GlobalConfig& getInstance();
    void loadConfig(const std::string& configFile);

    const ServiceData& getOrchestrator() const;
    const ServiceData& getFrameAnalytics() const;
    const ServiceData& getVideoPreProcessing() const;
    const ServiceData& getVideoPostProcessing() const;
    const ServiceData& getRedis() const;
    const DatabaseConfig& getPgDatabaseConfig() const;

private:
    GlobalConfig() = default;
    ~GlobalConfig() = default;
    GlobalConfig(const GlobalConfig&) = delete;
    GlobalConfig& operator=(const GlobalConfig&) = delete;

    ServiceData orchestrator;
    ServiceData frame_analytics;
    ServiceData video_pre_processing;
    ServiceData video_post_processing;

    ServiceData redis;

    DatabaseConfig pg_db;
};

} // namespace cfg