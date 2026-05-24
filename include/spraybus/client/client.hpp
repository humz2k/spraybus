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

  public:
    Client(std::string host, uint16_t port)
        : common::ClassLogger("client"), m_client(host, port), m_host(host),
          m_port(port) {}

    uint64_t get_topic_key(const std::string& topic) {
        auto it = m_topic_map.find(topic);
        if (it != m_topic_map.end()) {
            return it->second;
        }

        auto msg = m_protocol_constructor.topic_request(topic);

        networking::client::Client temporary_client(m_host, m_port);
        temporary_client.send(msg);
        auto packet = temporary_client.recv();
        protocol::Message response(packet.data());
        if (response.header().type() == protocol::Type::topic_response) {
            std::string topic_name = std::string(response.payload_as_string());
            uint64_t topic_key = response.header().topic_key();
            m_topic_map[topic_name] = topic_key;
            LOG_INFO(this->logger(), "Registered topic '{}' with key {}",
                     topic_name, topic_key);
            return topic_key;
        } else {
            throw std::runtime_error(
                "Unexpected response type: " +
                std::string(protocol::to_string(response.header().type())));
        }
    }

    void publish(const std::string& topic, std::span<const std::byte> payload) {
        uint64_t topic_key = get_topic_key(topic);
        auto msg = m_protocol_constructor.publish(topic_key, payload);
        m_client.send(msg);
    }

    void publish(uint64_t topic_key, std::span<const std::byte> payload) {
        auto msg = m_protocol_constructor.publish(topic_key, payload);
        m_client.send(msg);
    }

    void publish(const std::string& topic, std::string_view payload) {
        publish(topic, std::as_bytes(std::span<const char>(payload.data(),
                                                           payload.size())));
    }

    void publish(uint64_t topic_key, std::string_view payload) {
        publish(topic_key, std::as_bytes(std::span<const char>(
                               payload.data(), payload.size())));
    }

    void subscribe(const std::string& topic) {
        uint64_t topic_key = get_topic_key(topic);
        auto msg = m_protocol_constructor.subscribe(topic_key);
        m_client.send(msg);
    }

    template <typename Lambda> void process(Lambda&& lambda) {
        networking::client::Packet packet;
        if (m_client.process(packet)) {
            protocol::Message msg(packet.data());
            switch (msg.header().type()) {
            case protocol::Type::fanout: {
                std::string topic_name = "unknown";
                for (const auto& [name, key] : m_topic_map) {
                    if (key == msg.header().topic_key()) {
                        topic_name = name;
                        break;
                    }
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
