#include <spraybus/common/env.hpp>
#include <spraybus/common/interrupts.hpp>
#include <spraybus/networking/networking.hpp>
#include <spraybus/server/logging.hpp>
#include <spraybus/server/server.hpp>

#include <iostream>
#include <string>

int main() {
    auto port =
        std::stoi(spraybus::common::get_env_or("SPRAYBUS_PORT", "6767"));

    spraybus::server::init_logging();
    spraybus::networking::init();
    spraybus::server::Server server(port);
    spraybus::common::run_forever([&]() { server.process(); });
    return 0;
}
