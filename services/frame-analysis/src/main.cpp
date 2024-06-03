#include <crow.h>

#include "../../../utils/cfg/global_config.h"

#include "handlers/handlers_frw.h"

int main() {
    crow::SimpleApp app;

    handlers::BindYoloHandler(app);

    const auto& config = cfg::GlobalConfig::getInstance();
    const auto& app_config = config.getFrameAnalytics();
    app.port(app_config.port).multithreaded().run();

    return 0;
}