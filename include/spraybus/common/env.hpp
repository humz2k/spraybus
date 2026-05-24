#pragma once

/**
 * @file env.hpp
 * @brief Helpers for reading process environment variables.
 */

#include <string>

namespace spraybus::common {

/**
 * @brief Return an environment variable value or a caller-provided default.
 *
 * @param name Environment variable name.
 * @param default_value Value to return when @p name is not set.
 * @return The environment value when present; otherwise @p default_value.
 */
std::string get_env_or(std::string name, std::string default_value);

/**
 * @brief Return a required environment variable value.
 *
 * @param name Environment variable name.
 * @return The environment value.
 * @throws std::runtime_error when @p name is not set.
 */
std::string get_env_or_throw(std::string name);

} // namespace spraybus::common
