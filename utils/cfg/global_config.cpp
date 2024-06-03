#include "global_config.h"

namespace cfg {

std::string GlobalConfig::DatabaseConfig::getConnectionString() const {
    return "dbname=" + dbname + " user=" + user + " password=" + password + 
           " hostaddr=" + hostaddr + " port=" + std::to_string(port);
}

void GlobalConfig::loadConfig(const std::string& configFile) {
    std::ifstream file(configFile);
    if (file.is_open()) {
        try {
            crow::json::rvalue configData;
            file >> configData;

            crow::json::rvalue orchestratorData = configData["orchestrator"];
            orchestrator.host = orchestratorData["host"].s();
            orchestrator.port = orchestratorData["port"].i();

            crow::json::rvalue frameAnalyticsData = configData["frame_analytics"];
            frame_analytics.host = frameAnalyticsData["host"].s();
            frame_analytics.port = frameAnalyticsData["port"].i();

            crow::json::rvalue videoPreProcessingData = configData["video_pre_processing"];
            video_pre_processing.host = videoPreProcessingData["host"].s();
            video_pre_processing.port = videoPreProcessingData["port"].i();

            crow::json::rvalue videoPostProcessingData = configData["video_post_processing"];
            video_post_processing.host = videoPostProcessingData["host"].s();
            video_post_processing.port = videoPostProcessingData["port"].i();
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
    instance.loadConfig(CONFIG_FILE);
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