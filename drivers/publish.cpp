#include <spraybus/client/client.hpp>
#include <spraybus/common/interrupts.hpp>
#include <spraybus/common/logging.hpp>
#include <spraybus/networking/networking.hpp>

int main() {
    spraybus::common::init_logging();
    spraybus::networking::init();
    spraybus::client::Client client("localhost", 6767);
    auto topic_key = client.get_topic_key("test_topic");
    while (true) {
        for (int i = 0; i < 100; i++) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            client.process([&](const spraybus::protocol::Message& msg) {
                LOG_INFO(client.logger(), "message {}", msg.to_string());
            });
        }
        client.publish(topic_key, "Hello, world!");
    }
    return 0;
}
