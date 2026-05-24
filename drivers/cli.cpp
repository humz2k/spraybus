#include <spraybus/client/client.hpp>
#include <spraybus/common/interrupts.hpp>
#include <spraybus/common/logging.hpp>
#include <spraybus/networking/networking.hpp>

int main() {
    spraybus::common::init_logging();
    spraybus::networking::init();
    spraybus::client::Client client("localhost", 6767);
    client.subscribe("test_topic");
    auto logger = spraybus::common::create_logger("cli");
    spraybus::common::run_forever([&]() {
        client.process([&](const spraybus::protocol::Message& msg) {
            LOG_INFO(logger, "> topic={} message={}", msg.topic(),
                     msg.to_string());
        });
    });
    return 0;
}
