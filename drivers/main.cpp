#include <spraybus/common/interrupts.hpp>
#include <spraybus/common/logging.hpp>
#include <spraybus/networking/networking.hpp>
#include <spraybus/server/server.hpp>

#include <iostream>

int main() {
    spraybus::common::init_logging();
    spraybus::networking::init();
    spraybus::server::Server server(6767);
    spraybus::common::run_forever([&]() { server.process(); });
    return 0;
}
