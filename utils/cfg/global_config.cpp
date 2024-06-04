#include "global_config.h"

#include <sstream>
#include <fstream>
#include <exception>
#include <iostream>

#include <crow/json.h>

namespace cfg {

std::string GlobalConfig::DatabaseConfig::getConnectionString() const {
    return "dbname=" + dbname + " user=" + user + " password=" + password + 
           " hostaddr=" + hostaddr + " port=" + std::to_string(port);
}

void GlobalConfig::loadConfig(const std::string& configFile, const bool log_parsing) {
    std::ifstream file(configFile);
    if (file.is_open()) {
        try {
            std::stringstream buffer;
            buffer << file.rdbuf();
            auto json_str = buffer.str();
            auto configData = crow::json::load(json_str);
            if (!configData) {
                throw std::runtime_error("Invalid JSON in config file");
            }

            if (log_parsing) {
                std::cout << "Starting to parse config file\n";
            }

            auto orchestratorData = configData["orchestrator"];
            orchestrator.host = orchestratorData["host"].s();
            orchestrator.port = orchestratorData["port"].i();
            
            if (log_parsing) {
                std::cout << "Parsed orchestrator data\n";
                std::cout << "Host: " << orchestrator.host << "\n";
                std::cout << "Port: " << orchestrator.port << "\n";
            }

            auto frameAnalyticsData = configData["frame-analytics"];
            frame_analytics.host = frameAnalyticsData["host"].s();
            frame_analytics.port = frameAnalyticsData["port"].i();

            if (log_parsing) {
                std::cout << "Parsed frame-analytics data\n";
                std::cout << "Host: " << frame_analytics.host << "\n";
                std::cout << "Port: " << frame_analytics.port << "\n";
            }

            auto videoPreProcessingData = configData["video-pre-processing"];
            video_pre_processing.host = videoPreProcessingData["host"].s();
            video_pre_processing.port = videoPreProcessingData["port"].i();

            if (log_parsing) {
                std::cout << "Parsed video pre-processing data\n";
                std::cout << "Host: " << video_pre_processing.host << "\n";
                std::cout << "Port: " << video_pre_processing.port << "\n";
            }

            auto videoPostProcessingData = configData["video-post-processing"];
            video_post_processing.host = videoPostProcessingData["host"].s();
            video_post_processing.port = videoPostProcessingData["port"].i();

            if (log_parsing) {
                std::cout << "Parsed video post-processing data\n";
                std::cout << "Host: " << video_post_processing.host << "\n";
                std::cout << "Port: " << video_post_processing.port << "\n";
            }

            auto redisData = configData["redis"];
            redis.host = redisData["host"].s();
            redis.port = redisData["port"].i();

            if (log_parsing) {
                std::cout << "Parsed redis data\n";
                std::cout << "Host: " << redis.host << "\n";
                std::cout << "Port: " << redis.port << "\n";
            }

            auto pgDbData = configData["pg"];
            pg_db.dbname = pgDbData["database"].s();
            pg_db.user = pgDbData["user"].s();
            pg_db.password = pgDbData["password"].s();
            pg_db.hostaddr = pgDbData["host"].s();
            pg_db.port = pgDbData["port"].i();

            if (log_parsing) {
                std::cout << "Parsed PostgreSQL data\n";
                std::cout << "Database: " << pg_db.dbname << "\n";
                std::cout << "User: " << pg_db.user << "\n";
                std::cout << "Password: " << pg_db.password << "\n";
                std::cout << "Host: " << pg_db.hostaddr << "\n";
                std::cout << "Port: " << pg_db.port << "\n";
            }
        } catch (const std::exception& e) {
            std::cerr << "Error parsing config file: " << e.what() << std::endl;
        }
        file.close();
    } else {
        std::cerr << "Error opening config file: " << configFile << std::endl;
        throw std::runtime_error("Error opening config file");
    }
}

GlobalConfig& GlobalConfig::getInstance() {
    // TODO: Perhaps make it so it parses config every n seconds again
    static GlobalConfig instance;
    if (instance.orchestrator.host.empty()) {
        instance.loadConfig(CONFIG_FILE);
    }
    return instance;
}

const GlobalConfig::ServiceData& GlobalConfig::getOrchestrator() const {
    return orchestrator;
}

const GlobalConfig::ServiceData& GlobalConfig::getFrameAnalytics() const {
    return frame_analytics;
}

const GlobalConfig::ServiceData& GlobalConfig::getVideoPreProcessing() const {
    return video_pre_processing;
}

const GlobalConfig::ServiceData& GlobalConfig::getVideoPostProcessing() const {
    return video_post_processing;
}

const GlobalConfig::ServiceData& GlobalConfig::getRedis() const {
    return redis;
}

const GlobalConfig::DatabaseConfig& GlobalConfig::getPgDatabaseConfig() const {
    return pg_db;
}

} // namespace cfg