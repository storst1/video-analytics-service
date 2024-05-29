#include "handlers/save_video.h"

int main() {
    crow::SimpleApp app;

    handlers::BindSaveVideoHandler(app);

    app.port(8083).multithreaded().run();

    return 0;
}
