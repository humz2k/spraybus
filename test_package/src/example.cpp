#include <spraybus/client/client.hpp>
#include <spraybus/networking/networking.hpp>
#include <spraybus/networking/protocol.hpp>

#include <cstddef>
#include <cstdint>
#include <span>

int main() {
    spraybus::networking::init();

    using spraybus::networking::protocol::Header;
    using spraybus::networking::protocol::Message;

    spraybus::networking::protocol::Constructor constructor;

    constexpr uint64_t topic_key = 0x0102030405060708ULL;
    auto publish_packet = constructor.publish(topic_key, {});

    if (publish_packet.size() != sizeof(Header)) {
        return 1;
    }

    const std::byte expected_topic_key[] = {
        std::byte{0x01}, std::byte{0x02}, std::byte{0x03}, std::byte{0x04},
        std::byte{0x05}, std::byte{0x06}, std::byte{0x07}, std::byte{0x08},
    };
    const std::byte* wire_topic_key =
        publish_packet.data() + sizeof(Header) - sizeof(topic_key);

    for (std::size_t i = 0; i < sizeof(topic_key); ++i) {
        if (wire_topic_key[i] != expected_topic_key[i]) {
            return 2;
        }
    }

    Message publish_message{std::span<const std::byte>(publish_packet)};
    if (publish_message.header().topic_key() != topic_key) {
        return 3;
    }

    auto packet = constructor.topic_request("alerts");

    if (packet.size() <= sizeof(Header)) {
        return 4;
    }

    return 0;
}
