#include "migrations.h"

#include "../../../../utils/db/pg.h"
#include "../../../../utils/cfg/global_config.h"

namespace tasks {

/**
 * Runs the database migrations.
 */
void RunMigrations() {
    const auto& config = cfg::GlobalConfig::getInstance();
    const auto& pg_db = config.getPgDatabaseConfig();

    const std::string connection_str = pg_db.getConnectionString();
    const std::string migrations_dir = "../../../migrations";
    
    utils::db::ApplyMigrations(connection_str, migrations_dir);
}

}