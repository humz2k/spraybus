#pragma once

#include <spraybus/common/logging.hpp>
#include <spraybus/networking/server/server.hpp>
#include <spraybus/protocol/protocol.hpp>
#include <spraybus/topic/map.hpp>

#include <unordered_set>

namespace spraybus::server {

struct ClientData {
    std::unordered_set<uint64_t> subscribed_topics;
};

class Server : public networking::server::Server<Server, ClientData>,
               common::ClassLogger {
  public:
    using Base = networking::server::Server<Server, ClientData>;
    using Client = typename Base::Client;

  private:
    topic::Map m_topic_map;
    protocol::Constructor m_protocol_constructor;

  public:
    Server(int port);
    ~Server();
    void on_process() {}
    void on_connect(Client* client);
    void on_disconnect(Client* client);
    void on_message(Client* client, std::span<std::byte> message);
};

} // namespace spraybus::server
