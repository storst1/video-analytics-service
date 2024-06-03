#include <crow.h>

#include "../../../utils/cfg/global_config.h"

#include "handlers/save_video.h"

int main() {
    crow::SimpleApp app;

    handlers::BindSaveVideoHandler(app);

    const auto& config = cfg::GlobalConfig::getInstance();
    const auto& app_config = config.getVideoPostProcessing();
    app.port(app_config.port).multithreaded().run();

    return 0;
}
