#include <spraybus/client/client.hpp>

namespace spraybus::client {

Client::Client(std::string host, uint16_t port)
    : common::ClassLogger("client"), m_client(host, port), m_host(host),
      m_port(port) {}

uint64_t Client::get_topic_key(const std::string& topic) {
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
        m_inverse_topic_map[topic_key] = topic_name;
        LOG_INFO(this->logger(), "Registered topic '{}' with key {}",
                 topic_name, topic_key);
        return topic_key;
    } else {
        throw std::runtime_error(
            "Unexpected response type: " +
            std::string(protocol::to_string(response.header().type())));
    }
}

void Client::publish(const std::string& topic,
                     std::span<const std::byte> payload) {
    uint64_t topic_key = get_topic_key(topic);
    auto msg = m_protocol_constructor.publish(topic_key, payload);
    m_client.send(msg);
}

void Client::publish(uint64_t topic_key, std::span<const std::byte> payload) {
    auto msg = m_protocol_constructor.publish(topic_key, payload);
    m_client.send(msg);
}

void Client::publish(const std::string& topic, std::string_view payload) {
    publish(topic, std::as_bytes(
                       std::span<const char>(payload.data(), payload.size())));
}

void Client::publish(uint64_t topic_key, std::string_view payload) {
    publish(topic_key, std::as_bytes(std::span<const char>(payload.data(),
                                                           payload.size())));
}

void Client::subscribe(const std::string& topic) {
    uint64_t topic_key = get_topic_key(topic);
    auto msg = m_protocol_constructor.subscribe(topic_key);
    m_client.send(msg);
}

} // namespace spraybus::client
