#include <spraybus/networking/networking.hpp>

#include <stdexcept>

namespace spraybus::networking {
void init() {
    if (enet_initialize() != 0) {
        throw std::runtime_error("Failed to initialize ENet");
    }
    atexit(enet_deinitialize);
}
} // namespace spraybus::networking
