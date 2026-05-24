#pragma once

#include <enet/enet.h>

#include <cstdint>
#include <optional>
#include <span>
#include <string>

namespace spraybus::networking::client {

class Packet {
  private:
    ENetPacket* m_packet = nullptr;

  public:
    Packet(ENetPacket* packet) : m_packet(packet) {}
    Packet() {}
    Packet(const Packet&) = delete;
    Packet& operator=(const Packet&) = delete;
    Packet(Packet&& other) noexcept : m_packet(other.m_packet) {
        other.m_packet = nullptr;
    }
    Packet& operator=(Packet&& other) noexcept {
        if (this != &other) {
            if (m_packet)
                enet_packet_destroy(m_packet);
            m_packet = other.m_packet;
            other.m_packet = nullptr;
        }
        return *this;
    }

    ~Packet() {
        if (m_packet)
            enet_packet_destroy(m_packet);
    }

    std::span<std::byte> data() const {
        return std::span<std::byte>(
            reinterpret_cast<std::byte*>(m_packet->data), m_packet->dataLength);
    }
};

class Client {
  private:
    ENetHost* m_client;
    ENetAddress m_address;
    ENetEvent m_event;
    ENetPeer* m_peer;

  public:
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

    void send(std::span<const std::byte> data) {
        ENetPacket* packet = enet_packet_create(data.data(), data.size(),
                                                ENET_PACKET_FLAG_RELIABLE);
        enet_peer_send(m_peer, 0, packet);
        enet_host_flush(m_client);
    }
};

} // namespace spraybus::networking::client
