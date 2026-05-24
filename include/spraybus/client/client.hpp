#pragma once

/**
 * @file client.hpp
 * @brief High-level spraybus publishing and subscription client.
 */

#include <spraybus/common/logging.hpp>
#include <spraybus/networking/client/client.hpp>
#include <spraybus/protocol/protocol.hpp>

#include <cstdint>
#include <span>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace spraybus::client {

/**
 * @brief High-level client for resolving topics, publishing, and receiving
 * fanout messages.
 *
 * The client maintains a local topic-name cache. Topic names are resolved
 * through the server before publish or subscribe operations that use strings.
 */
class Client : public common::ClassLogger {
  private:
    networking::client::Client m_client;
    std::string m_host;
    uint16_t m_port;
    protocol::Constructor m_protocol_constructor;
    std::unordered_map<std::string, uint64_t> m_topic_map;
    std::unordered_map<uint64_t, std::string> m_inverse_topic_map;

  public:
    /**
     * @brief Connect to a spraybus server.
     *
     * @param host Server hostname or IP address.
     * @param port Server UDP port.
     * @throws std::runtime_error when the underlying ENet connection fails.
     */
    Client(std::string host, uint16_t port);

    /**
     * @brief Resolve a topic name to its numeric topic key.
     *
     * Results are cached in the client after the first server lookup.
     *
     * @param topic Topic name.
     * @return Server-assigned topic key.
     * @throws std::runtime_error when the server returns an unexpected message.
     */
    uint64_t get_topic_key(const std::string& topic);

    /**
     * @brief Publish binary payload bytes to a named topic.
     *
     * @param topic Topic name.
     * @param payload Payload bytes.
     */
    void publish(const std::string& topic, std::span<const std::byte> payload);

    /**
     * @brief Publish binary payload bytes to an already resolved topic key.
     *
     * @param topic_key Server-assigned topic key.
     * @param payload Payload bytes.
     */
    void publish(uint64_t topic_key, std::span<const std::byte> payload);

    /**
     * @brief Publish a string payload to a named topic.
     *
     * @param topic Topic name.
     * @param payload String payload. It is sent as raw bytes without a null
     * terminator.
     */
    void publish(const std::string& topic, std::string_view payload);

    /**
     * @brief Publish a string payload to an already resolved topic key.
     *
     * @param topic_key Server-assigned topic key.
     * @param payload String payload. It is sent as raw bytes without a null
     * terminator.
     */
    void publish(uint64_t topic_key, std::string_view payload);

    /**
     * @brief Subscribe to a named topic.
     *
     * @param topic Topic name.
     */
    void subscribe(const std::string& topic);

    /**
     * @brief Poll once for a fanout message and invoke a callback if one
     * exists.
     *
     * @tparam Lambda Callable receiving `const protocol::Message&`.
     * @param lambda Callback invoked for each received fanout message.
     * @throws std::runtime_error when the underlying connection disconnects.
     */
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
