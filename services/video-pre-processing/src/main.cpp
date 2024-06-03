#include <crow.h>

#include "../../../utils/cfg/global_config.h"

#include "handlers/handlers_frw.h"

int main() {
    crow::SimpleApp app;

    handlers::BindProcessVideoHandler(app);

    const auto& config = cfg::GlobalConfig::getInstance();
    const auto& app_config = config.getVideoPreProcessing();
    app.port(app_config.port).multithreaded().run();

    return 0;
}
