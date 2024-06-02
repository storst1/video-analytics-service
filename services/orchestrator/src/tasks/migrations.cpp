#include "migrations.h"

#include "../../../../utils/db/pg.h"

namespace tasks {

/**
 * Runs the database migrations using the provided connection string.
 *
 * @param connection_str The connection string for the database.
 */
void RunMigrations(const std::string& connection_str) {
    const std::string migrations_dir = "../../../migrations";
    utils::db::ApplyMigrations(connection_str, migrations_dir);
}

}