#include <spraybus/server/server.hpp>

namespace spraybus::server {

Server::Server(int port) : Base(port), common::ClassLogger("server") {
    LOG_INFO(this->logger(), "Server started on port {}", port);
}

Server::~Server() { LOG_INFO(this->logger(), "Server shutting down"); }

void Server::on_connect(Client* client) {
    LOG_INFO(this->logger(), "Client {} connected", client->id());
}

void Server::on_disconnect(Client* client) {
    LOG_INFO(this->logger(), "Client {} disconnected", client->id());
}

void Server::on_message(Client* client, std::span<std::byte> message) {
    protocol::Message msg(message);
    LOG_DEBUG(this->logger(), "Received message from client {}: {}",
              client->id(), msg.to_string());
    switch (msg.header().type()) {
    case protocol::Type::topic_request: {
        std::string topic_name = std::string(msg.payload_as_string());
        uint64_t topic_key = m_topic_map.get_or_create(topic_name);
        auto response_payload =
            m_protocol_constructor.topic_response(topic_key, topic_name);
        client->send(response_payload);
        LOG_INFO(this->logger(), "Registered topic '{}' with key {}",
                 topic_name, topic_key);
    } break;
    case protocol::Type::subscribe: {
        uint64_t topic_key = msg.header().topic_key();
        client->data().subscribed_topics.insert(topic_key);
        LOG_INFO(this->logger(), "Client {} subscribed to topic key {}",
                 client->id(), topic_key);
    } break;
    case protocol::Type::unsubscribe: {
        uint64_t topic_key = msg.header().topic_key();
        client->data().subscribed_topics.erase(topic_key);
        LOG_INFO(this->logger(), "Client {} unsubscribed from topic key {}",
                 client->id(), topic_key);
    } break;
    case protocol::Type::publish: {
        uint64_t topic_key = msg.header().topic_key();
        auto payload = msg.payload();
        auto fanout_payload = m_protocol_constructor.fanout(topic_key, payload);
        for (auto* other_client : this->clients()) {
            if (other_client->id() != client->id() &&
                other_client->data().subscribed_topics.contains(topic_key)) {
                other_client->send(fanout_payload);
                LOG_DEBUG(this->logger(),
                          "Forwarded message from client {} to client {} "
                          "for topic key {}",
                          client->id(), other_client->id(), topic_key);
            }
        }
    } break;
    default:
        LOG_WARNING(this->logger(), "Unhandled message type: {}",
                    protocol::to_string(msg.header().type()));
        break;
    }
}
} // namespace spraybus::server
