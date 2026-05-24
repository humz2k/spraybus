#pragma once

#include <enet/enet.h>

#include <algorithm>
#include <cstdint>
#include <span>
#include <unordered_map>
#include <vector>

namespace spraybus::networking::server {

template <typename ClientData> class Client {
  private:
    uint64_t m_id;
    ENetPeer* m_peer;
    ENetHost* m_server;
    ClientData m_data;

  public:
    Client(uint64_t id, ENetPeer* peer, ENetHost* server)
        : m_id(id), m_peer(peer), m_server(server) {}

    uint64_t id() const { return m_id; }

    ClientData& data() { return m_data; }

    const ClientData& data() const { return m_data; }

    void send(std::span<const std::byte> message) {
        ENetPacket* packet = enet_packet_create(message.data(), message.size(),
                                                ENET_PACKET_FLAG_RELIABLE);
        enet_peer_send(m_peer, 0, packet);
        enet_host_flush(m_server);
    }
};

template <typename Derived, typename ClientData> class Server {
  public:
    using Client = Client<ClientData>;

  private:
    ENetAddress m_address;
    ENetHost* m_server = nullptr;

    std::unordered_map<uint64_t, Client> m_client_map;
    std::vector<Client*> m_clients;
    uint64_t m_next_client_id = 1;

    Derived* derived() { return static_cast<Derived*>(this); }
    const Derived* derived() const { return static_cast<const Derived*>(this); }

    Client* create_client(ENetPeer* peer) {
        uint64_t client_id = m_next_client_id++;
        auto [it, inserted] =
            m_client_map.emplace(client_id, Client(client_id, peer, m_server));
        if (!inserted) {
            throw std::runtime_error("Failed to create client with ID: " +
                                     std::to_string(client_id));
        }
        auto* client = &it->second;
        m_clients.push_back(client);
        return client;
    }

    void destroy_client(Client* client) {
        m_client_map.erase(client->id());
        std::erase(m_clients, client);
    }

  public:
    Server(int port) {
        m_address.host = ENET_HOST_ANY;
        m_address.port = port;
        m_server = enet_host_create(&m_address, 8, 2, 0, 0);
        if (m_server == nullptr) {
            throw std::runtime_error("Failed to create ENet server");
        }
    }

    ~Server() { enet_host_destroy(m_server); }

    void process() {
        derived()->on_process();
        ENetEvent event;
        while (enet_host_service(m_server, &event, 0) > 0) {
            switch (event.type) {
            case ENET_EVENT_TYPE_CONNECT:
                event.peer->data = create_client(event.peer);
                derived()->on_connect(static_cast<Client*>(event.peer->data));
                break;
            case ENET_EVENT_TYPE_RECEIVE:
                derived()->on_message(
                    static_cast<Client*>(event.peer->data),
                    std::span<std::byte>(
                        reinterpret_cast<std::byte*>(event.packet->data),
                        event.packet->dataLength));
                enet_packet_destroy(event.packet);
                break;
            case ENET_EVENT_TYPE_DISCONNECT:
                derived()->on_disconnect(
                    static_cast<Client*>(event.peer->data));
                destroy_client(static_cast<Client*>(event.peer->data));
                break;
            default:
                break;
            }
        }
    }

    void broadcast(std::span<const std::byte> message) {
        ENetPacket* packet = enet_packet_create(message.data(), message.size(),
                                                ENET_PACKET_FLAG_RELIABLE);
        enet_host_broadcast(m_server, 0, packet);
        enet_host_flush(m_server);
    }

    std::span<Client*> clients() { return m_clients; }
    std::span<const Client*> clients() const { return m_clients; }
};

} // namespace spraybus::networking::server
