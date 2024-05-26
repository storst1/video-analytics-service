#include <iostream>

#include <crow.h>

#include "handlers/handlers_frw.h"

int main()
{
    crow::SimpleApp app;

    handlers::BindSubmitVideoHandler(app);
    handlers::BindStatusHandler(app);

    app.port(8080).multithreaded().run();

    return 0;
}
