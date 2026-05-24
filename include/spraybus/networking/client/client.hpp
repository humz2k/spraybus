#pragma once

/**
 * @file client.hpp
 * @brief Low-level ENet client wrapper used by spraybus clients.
 */

#include <enet/enet.h>

#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <stdexcept>
#include <string>

namespace spraybus::networking::client {

/**
 * @brief RAII wrapper for an ENet packet received by a client.
 *
 * Packet owns the underlying ENetPacket and destroys it when the wrapper is
 * destroyed or reassigned.
 */
class Packet {
  private:
    ENetPacket* m_packet = nullptr;

  public:
    /**
     * @brief Take ownership of a received ENet packet.
     *
     * @param packet Packet pointer returned by ENet.
     */
    Packet(ENetPacket* packet) : m_packet(packet) {}

    /**
     * @brief Construct an empty packet wrapper.
     */
    Packet() {}

    /**
     * @brief Copying is disabled because Packet has unique ownership.
     */
    Packet(const Packet&) = delete;

    /**
     * @brief Copy assignment is disabled because Packet has unique ownership.
     */
    Packet& operator=(const Packet&) = delete;

    /**
     * @brief Move a packet wrapper, transferring ownership.
     */
    Packet(Packet&& other) noexcept : m_packet(other.m_packet) {
        other.m_packet = nullptr;
    }

    /**
     * @brief Move-assign a packet wrapper, replacing any owned packet.
     */
    Packet& operator=(Packet&& other) noexcept {
        if (this != &other) {
            if (m_packet)
                enet_packet_destroy(m_packet);
            m_packet = other.m_packet;
            other.m_packet = nullptr;
        }
        return *this;
    }

    /**
     * @brief Destroy any owned ENet packet.
     */
    ~Packet() {
        if (m_packet)
            enet_packet_destroy(m_packet);
    }

    /**
     * @brief Return a mutable byte view over the packet payload.
     *
     * @return Packet data bytes.
     */
    std::span<std::byte> data() const {
        return std::span<std::byte>(
            reinterpret_cast<std::byte*>(m_packet->data), m_packet->dataLength);
    }
};

/**
 * @brief Low-level client connection to a spraybus server over ENet.
 */
class Client {
  private:
    ENetHost* m_client;
    ENetAddress m_address;
    ENetEvent m_event;
    ENetPeer* m_peer;

  public:
    /**
     * @brief Connect to a server.
     *
     * @param host Hostname or IP address.
     * @param port UDP port.
     * @throws std::runtime_error when host creation, connection, or handshake
     * fails.
     */
    Client(std::string host, uint16_t port) {
        m_client = enet_host_create(NULL, 1, 2, 0, 0);
        if (m_client == NULL) {
            throw std::runtime_error("An error occurred while trying to create "
                                     "an ENet client host.");
        }

        enet_address_set_host(&m_address, host.c_str());
        m_address.port = port;

        m_peer = enet_host_connect(m_client, &m_address, 2, 0);

        if (m_peer == NULL) {
            throw std::runtime_error(
                "No available peers for initiating an ENet connection.");
        }

        /* Wait up to 5 seconds for the connection attempt to succeed. */
        if (!(enet_host_service(m_client, &m_event, 5000) > 0 &&
              m_event.type == ENET_EVENT_TYPE_CONNECT)) {
            enet_peer_reset(m_peer);
            throw std::runtime_error("Connection to localhost:6767 failed.");
        }
    }

    /**
     * @brief Disconnect from the server and release ENet resources.
     */
    ~Client() {
        enet_peer_disconnect(m_peer, 0);
        while (enet_host_service(m_client, &m_event, 3000) > 0) {
            if (m_event.type == ENET_EVENT_TYPE_DISCONNECT) {
                break;
            } else if (m_event.type == ENET_EVENT_TYPE_RECEIVE) {
                enet_packet_destroy(m_event.packet);
            }
        }
        enet_host_destroy(m_client);
    }

    /**
     * @brief Poll once for a received packet.
     *
     * @param packet Output packet assigned when a packet is received.
     * @return true when @p packet was assigned; false when no event is ready.
     * @throws std::runtime_error when the server disconnects.
     */
    bool process(Packet& packet) {
        if (enet_host_service(m_client, &m_event, 0) > 0) {
            switch (m_event.type) {
            case ENET_EVENT_TYPE_RECEIVE:
                packet = Packet(m_event.packet);
                return true;
            case ENET_EVENT_TYPE_DISCONNECT:
                throw std::runtime_error("Disconnected from server.");
            default:
                break;
            }
        }
        return false;
    }

    /**
     * @brief Wait for a received packet.
     *
     * @param timeout_ms Maximum time to wait in milliseconds.
     * @return Received packet.
     * @throws std::runtime_error on timeout or disconnect.
     */
    Packet recv(uint32_t timeout_ms = 1000) {
        if (enet_host_service(m_client, &m_event, timeout_ms) > 0) {
            switch (m_event.type) {
            case ENET_EVENT_TYPE_RECEIVE:
                return Packet(m_event.packet);
            case ENET_EVENT_TYPE_DISCONNECT:
                throw std::runtime_error("Disconnected from server.");
            default:
                break;
            }
        }
        throw std::runtime_error("Timeout while waiting for packet.");
    }

    /**
     * @brief Send bytes to the connected peer reliably.
     *
     * @param data Packet bytes to send.
     */
    void send(std::span<const std::byte> data) {
        ENetPacket* packet = enet_packet_create(data.data(), data.size(),
                                                ENET_PACKET_FLAG_RELIABLE);
        enet_peer_send(m_peer, 0, packet);
        enet_host_flush(m_client);
    }
};

} // namespace spraybus::networking::client
