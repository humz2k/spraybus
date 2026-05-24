#include <spraybus/common/env.hpp>

#include <cstdlib>
#include <stdexcept>

namespace spraybus::common {

std::string get_env_or(std::string name, std::string default_value) {
    if (const char* value = std::getenv(name.c_str())) {
        return value;
    }
    return std::string(default_value);
}

std::string get_env_or_throw(std::string name) {
    if (const char* value = std::getenv(name.c_str())) {
        return value;
    }
    throw std::runtime_error("Missing required environment variable: " + name);
}

} // namespace spraybus::common
