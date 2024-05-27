#include "handlers/handlers_frw.h"

int main() {
    crow::SimpleApp app;

    handlers::BindYoloHandler(app);

    app.port(8082).multithreaded().run();
}