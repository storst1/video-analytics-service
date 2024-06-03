#include <iostream>

#include <crow.h>

#include "handlers/handlers_frw.h"
#include "tasks/migrations.h"

int main()
{
    tasks::RunMigrations();

    crow::SimpleApp app;

    handlers::BindSubmitVideoHandler(app);
    handlers::BindStatusHandler(app);

    app.port(8080).multithreaded().run();

    return 0;
}
