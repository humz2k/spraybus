#include <spraybus/client/client.hpp>
#include <spraybus/networking/networking.hpp>
#include <spraybus/networking/protocol.hpp>

#include <chrono>
#include <iostream>
#include <thread>

int main() {
    spraybus::networking::init();

    spraybus::client::Client client("localhost", 6767);
    client.publish("test_topic", "hello from spraybus-client-example");
    client.subscribe("test_topic");

    while (true) {
        client.process(
            [](const spraybus::networking::protocol::Message& message) {
                std::cout << message.payload_as_string() << '\n';
                std::cout.flush();
            });
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}
