#pragma once

#include <string>

namespace spraybus::common {

std::string get_env_or(std::string name, std::string default_value);

std::string get_env_or_throw(std::string name);

} // namespace spraybus::common
