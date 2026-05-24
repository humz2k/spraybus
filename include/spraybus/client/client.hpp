#pragma once

#include <spraybus/common/logging.hpp>
#include <spraybus/networking/client/client.hpp>
#include <spraybus/protocol/protocol.hpp>

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

namespace spraybus::client {

class Client : public common::ClassLogger {
  private:
    networking::client::Client m_client;
    std::string m_host;
    uint16_t m_port;
    protocol::Constructor m_protocol_constructor;
    std::unordered_map<std::string, uint64_t> m_topic_map;
    std::unordered_map<uint64_t, std::string> m_inverse_topic_map;

  public:
    Client(std::string host, uint16_t port);

    uint64_t get_topic_key(const std::string& topic);

    void publish(const std::string& topic, std::span<const std::byte> payload);

    void publish(uint64_t topic_key, std::span<const std::byte> payload);

    void publish(const std::string& topic, std::string_view payload);

    void publish(uint64_t topic_key, std::string_view payload);

    void subscribe(const std::string& topic);

    template <typename Lambda> void process(Lambda&& lambda) {
        networking::client::Packet packet;
        if (m_client.process(packet)) {
            protocol::Message msg(packet.data());
            switch (msg.header().type()) {
            case protocol::Type::fanout: {
                std::string_view topic_name = "unknown";
                auto it = m_inverse_topic_map.find(msg.header().topic_key());
                if (it != m_inverse_topic_map.end()) {
                    topic_name = it->second;
                }
                msg.set_topic(topic_name);
                lambda(msg);
            } break;
            default:
                LOG_WARNING(this->logger(), "Unhandled message type: {}",
                            protocol::to_string(msg.header().type()));
                break;
            }
        }
    }
};

} // namespace spraybus::client
