#pragma once

#include <string>

namespace tasks {

void RunMigrations(const std::string& connection_str);

}