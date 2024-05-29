#include "migrations.h"

#include "../../../../utils/db/pg.h"

namespace tasks {

void RunMigrations(const std::string& connection_str) {
    const std::string migrations_dir = "../../../migrations";
    utils::db::ApplyMigrations(connection_str, migrations_dir);
}

}