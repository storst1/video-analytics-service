#include "handlers/handlers_frw.h"

int main() {
    crow::SimpleApp app;

    handlers::BindProcessVideoHandler(app);

    app.port(8081).multithreaded().run();
}
