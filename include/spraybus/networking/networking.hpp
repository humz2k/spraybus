#pragma once

/**
 * @file networking.hpp
 * @brief Process-wide ENet initialization helpers.
 */

#include <enet/enet.h>

namespace spraybus::networking {

/**
 * @brief Initialize ENet and register process-exit cleanup.
 *
 * Call this before creating any spraybus networking clients or servers.
 *
 * @throws std::runtime_error when ENet initialization fails.
 */
void init();

} // namespace spraybus::networking
