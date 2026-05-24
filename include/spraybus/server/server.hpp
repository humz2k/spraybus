#pragma once

/**
 * @file server.hpp
 * @brief High-level spraybus pub/sub server.
 */

#include <spraybus/networking/protocol.hpp>
#include <spraybus/networking/server/server.hpp>
#include <spraybus/server/logging.hpp>
#include <spraybus/server/topic_map.hpp>

#include <cstdint>
#include <span>
#include <unordered_set>

namespace spraybus::server {

/**
 * @brief Per-client subscription state stored by the server.
 */
struct ClientData {
    /**
     * @brief Topic keys this client is currently subscribed to.
     */
    std::unordered_set<uint64_t> subscribed_topics;
};

/**
 * @brief Concrete spraybus server implementation.
 *
 * The server resolves topic names to numeric keys, tracks client
 * subscriptions, and forwards published payloads to subscribed peers.
 */
class Server : public networking::server::Server<Server, ClientData>,
               ClassLogger {
  public:
    /**
     * @brief Low-level CRTP server base type.
     */
    using Base = networking::server::Server<Server, ClientData>;

    /**
     * @brief Connected client type used by this server.
     */
    using Client = typename Base::Client;

  private:
    TopicMap m_topic_map;
    networking::protocol::Constructor m_protocol_constructor;

  public:
    /**
     * @brief Start a server bound to a UDP port.
     *
     * @param port UDP port in the range 0..65535.
     */
    Server(int port);

    /**
     * @brief Log server shutdown.
     */
    ~Server();

    /**
     * @brief Per-tick callback used by the networking base.
     */
    void on_process() {}

    /**
     * @brief Handle a newly connected client.
     *
     * @param client Connected client.
     */
    void on_connect(Client* client);

    /**
     * @brief Handle a client disconnect.
     *
     * @param client Disconnecting client.
     */
    void on_disconnect(Client* client);

    /**
     * @brief Handle a packet received from a client.
     *
     * Malformed packets shorter than the protocol header are logged and
     * ignored.
     *
     * @param client Sending client.
     * @param message Packet bytes.
     */
    void on_message(Client* client, std::span<std::byte> message);
};

} // namespace spraybus::server
