#include <crow.h>

#include "../../../utils/cfg/global_config.h"

#include "handlers/handlers_frw.h"
#include "tasks/migrations.h"

int main()
{
    tasks::RunMigrations();

    crow::SimpleApp app;

    handlers::BindSubmitVideoHandler(app);
    handlers::BindStatusHandler(app);
    handlers::BindStopHandler(app);

    const auto& config = cfg::GlobalConfig::getInstance();
    const auto& app_config = config.getOrchestrator();
    app.port(app_config.port).multithreaded().run();

    return 0;
}
