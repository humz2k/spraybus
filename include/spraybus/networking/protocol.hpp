#pragma once

/**
 * @file protocol.hpp
 * @brief Wire protocol primitives for spraybus messages.
 */

#include <cstddef>
#include <cstdint>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace spraybus::networking::protocol {

/**
 * @brief Identifies which kind of node created a message.
 */
enum class Origin : uint8_t {
    unknown = 0x00, ///< Origin is unset or not recognized.
    client = 0x01,  ///< Message originated from a client.
    server = 0x02,  ///< Message originated from a server.
    leaf = 0x03,    ///< Reserved for future leaf-node forwarding.
};

/**
 * @brief Convert an Origin value to a stable display string.
 *
 * @param origin Origin value.
 * @return Human-readable origin name.
 */
inline std::string_view to_string(Origin origin) {
    switch (origin) {
    case Origin::unknown:
        return "unknown";
    case Origin::client:
        return "client";
    case Origin::server:
        return "server";
    case Origin::leaf:
        return "leaf";
    default:
        return "invalid";
    }
}

/**
 * @brief Protocol version encoded in every message header.
 */
enum class Version : uint16_t {
    v1 = 0x01, ///< Initial spraybus protocol version.
};

/**
 * @brief Convert a Version value to a stable display string.
 *
 * @param version Version value.
 * @return Human-readable version name.
 */
inline std::string_view to_string(Version version) {
    switch (version) {
    case Version::v1:
        return "v1";
    default:
        return "invalid";
    }
}

/**
 * @brief Message type encoded in every protocol header.
 */
enum class Type : uint8_t {
    unknown = 0x00,        ///< Type is unset or not recognized.
    topic_request = 0x01,  ///< Client asks the server to resolve a topic name.
    topic_response = 0x02, ///< Server returns the numeric key for a topic.
    publish = 0x03,        ///< Client publishes a payload for a topic key.
    subscribe = 0x04,      ///< Client subscribes to a topic key.
    unsubscribe = 0x05,    ///< Client unsubscribes from a topic key.
    fanout = 0x06,         ///< Server forwards a published payload.
};

/**
 * @brief Convert a Type value to a stable display string.
 *
 * @param type Type value.
 * @return Human-readable message type name.
 */
inline std::string_view to_string(Type type) {
    switch (type) {
    case Type::unknown:
        return "unknown";
    case Type::topic_request:
        return "topic_request";
    case Type::topic_response:
        return "topic_response";
    case Type::publish:
        return "publish";
    case Type::subscribe:
        return "subscribe";
    case Type::unsubscribe:
        return "unsubscribe";
    case Type::fanout:
        return "fanout";
    default:
        return "invalid";
    }
}

/**
 * @brief Fixed-size protocol header carried at the start of every packet.
 *
 * The header is currently 16 bytes and is followed by an optional payload. The
 * topic key is zero for requests that identify a topic by name in the payload.
 */
struct [[gnu::packed]] Header {
  public:
    /**
     * @brief Current protocol version emitted by Header factory helpers.
     */
    constexpr static Version current_version = Version::v1;

  private:
    const uint32_t m_reserved_ = 0;
    const Version m_version_ = current_version;
    const Origin m_origin_;
    const Type m_type_;
    const uint64_t m_topic_key_;

    Header(Origin origin, Type type, uint64_t topic_key)
        : m_origin_(origin), m_type_(type), m_topic_key_(topic_key) {}

  public:
    /**
     * @brief Create a topic-name lookup request header.
     *
     * @param origin Origin to encode in the header.
     * @return Header for a topic request.
     */
    static Header topic_request(Origin origin = Origin::client) {
        return Header(origin, Type::topic_request, 0);
    }

    /**
     * @brief Create a topic-name lookup response header.
     *
     * @param topic_key Numeric key assigned to the requested topic.
     * @param origin Origin to encode in the header.
     * @return Header for a topic response.
     */
    static Header topic_response(uint64_t topic_key,
                                 Origin origin = Origin::server) {
        return Header(origin, Type::topic_response, topic_key);
    }

    /**
     * @brief Create a client publish header.
     *
     * @param topic_key Topic key to publish to.
     * @param origin Origin to encode in the header.
     * @return Header for a publish message.
     */
    static Header publish(uint64_t topic_key, Origin origin = Origin::client) {
        return Header(origin, Type::publish, topic_key);
    }

    /**
     * @brief Create a subscribe header.
     *
     * @param topic_key Topic key to subscribe to.
     * @param origin Origin to encode in the header.
     * @return Header for a subscribe message.
     */
    static Header subscribe(uint64_t topic_key,
                            Origin origin = Origin::client) {
        return Header(origin, Type::subscribe, topic_key);
    }

    /**
     * @brief Create an unsubscribe header.
     *
     * @param topic_key Topic key to unsubscribe from.
     * @param origin Origin to encode in the header.
     * @return Header for an unsubscribe message.
     */
    static Header unsubscribe(uint64_t topic_key,
                              Origin origin = Origin::client) {
        return Header(origin, Type::unsubscribe, topic_key);
    }

    /**
     * @brief Create a server fanout header.
     *
     * @param topic_key Topic key associated with the forwarded payload.
     * @param origin Origin to encode in the header.
     * @return Header for a fanout message.
     */
    static Header fanout(uint64_t topic_key, Origin origin = Origin::server) {
        return Header(origin, Type::fanout, topic_key);
    }

    /**
     * @brief Return the protocol version.
     */
    Version version() const { return m_version_; }

    /**
     * @brief Return the origin node kind.
     */
    Origin origin() const { return m_origin_; }

    /**
     * @brief Return the message type.
     */
    Type type() const { return m_type_; }

    /**
     * @brief Return the topic key encoded in the header.
     */
    uint64_t topic_key() const { return m_topic_key_; }

    /**
     * @brief Render the header as a diagnostic string.
     *
     * @return Human-readable header description.
     */
    std::string to_string() const {
        return "Header { version: " +
               std::string(
                   ::spraybus::networking::protocol::to_string(m_version_)) +
               ", origin: " +
               std::string(
                   ::spraybus::networking::protocol::to_string(m_origin_)) +
               ", type: " +
               std::string(
                   ::spraybus::networking::protocol::to_string(m_type_)) +
               ", topic_key: " + std::to_string(m_topic_key_) + " }";
    }
};

static_assert(sizeof(Header) == 16, "Header must be exactly 16 bytes");

/**
 * @brief Reusable message buffer builder for protocol packets.
 *
 * Constructor owns one internal buffer. Each construction method overwrites
 * that buffer and returns a span valid until the next call on the same object.
 */
class Constructor {
  private:
    std::vector<std::byte> m_buffer;

  public:
    /**
     * @brief Create an empty protocol message constructor.
     */
    Constructor() = default;

    /**
     * @brief Construct a raw packet from a header and optional payload.
     *
     * @param header Protocol header.
     * @param payload Optional payload bytes.
     * @return Mutable bytes of the constructed packet.
     */
    std::span<std::byte> construct(const Header& header,
                                   std::span<const std::byte> payload = {}) {
        m_buffer.clear();
        const std::byte* header_bytes =
            reinterpret_cast<const std::byte*>(&header);
        m_buffer.insert(m_buffer.end(), header_bytes,
                        header_bytes + sizeof(header));
        m_buffer.insert(m_buffer.end(), payload.begin(), payload.end());
        return std::span<std::byte>(m_buffer);
    }

    /**
     * @brief Construct a topic request packet.
     *
     * @param topic_name Topic name to resolve.
     * @param origin Origin to encode in the packet header.
     * @return Packet bytes valid until the next construction call.
     */
    std::span<std::byte> topic_request(std::string_view topic_name,
                                       Origin origin = Origin::client) {
        Header header = Header::topic_request(origin);
        return construct(header, std::as_bytes(std::span<const char>(
                                     topic_name.data(), topic_name.size())));
    }

    /**
     * @brief Construct a topic response packet.
     *
     * @param topic_key Numeric key assigned to the topic.
     * @param topic_name Topic name being resolved.
     * @param origin Origin to encode in the packet header.
     * @return Packet bytes valid until the next construction call.
     */
    std::span<std::byte> topic_response(uint64_t topic_key,
                                        std::string_view topic_name,
                                        Origin origin = Origin::server) {
        Header header = Header::topic_response(topic_key, origin);
        return construct(header, std::as_bytes(std::span<const char>(
                                     topic_name.data(), topic_name.size())));
    }

    /**
     * @brief Construct a publish packet.
     *
     * @param topic_key Topic key to publish to.
     * @param payload Payload bytes.
     * @param origin Origin to encode in the packet header.
     * @return Packet bytes valid until the next construction call.
     */
    std::span<std::byte> publish(uint64_t topic_key,
                                 std::span<const std::byte> payload,
                                 Origin origin = Origin::client) {
        Header header = Header::publish(topic_key, origin);
        return construct(header, payload);
    }

    /**
     * @brief Construct a subscribe packet.
     *
     * @param topic_key Topic key to subscribe to.
     * @param origin Origin to encode in the packet header.
     * @return Packet bytes valid until the next construction call.
     */
    std::span<std::byte> subscribe(uint64_t topic_key,
                                   Origin origin = Origin::client) {
        Header header = Header::subscribe(topic_key, origin);
        return construct(header);
    }

    /**
     * @brief Construct an unsubscribe packet.
     *
     * @param topic_key Topic key to unsubscribe from.
     * @param origin Origin to encode in the packet header.
     * @return Packet bytes valid until the next construction call.
     */
    std::span<std::byte> unsubscribe(uint64_t topic_key,
                                     Origin origin = Origin::client) {
        Header header = Header::unsubscribe(topic_key, origin);
        return construct(header);
    }

    /**
     * @brief Construct a fanout packet.
     *
     * @param topic_key Topic key associated with the forwarded payload.
     * @param payload Payload bytes.
     * @param origin Origin to encode in the packet header.
     * @return Packet bytes valid until the next construction call.
     */
    std::span<std::byte> fanout(uint64_t topic_key,
                                std::span<const std::byte> payload,
                                Origin origin = Origin::server) {
        Header header = Header::fanout(topic_key, origin);
        return construct(header, payload);
    }
};

/**
 * @brief Non-owning view over a received protocol packet.
 *
 * Message points directly into the byte span passed to the constructor. The
 * caller must keep those bytes alive while using the Message.
 */
class Message {
  private:
    const Header* m_header;
    std::span<const std::byte> m_payload;
    std::string_view m_topic;

    const Header* parse_header(std::span<const std::byte> data) {
        if (data.size() < sizeof(Header)) {
            throw std::runtime_error("Data too short to contain header");
        }
        return reinterpret_cast<const Header*>(data.data());
    }

  public:
    /**
     * @brief Parse a protocol message view from packet bytes.
     *
     * @param data Packet bytes containing a Header followed by payload bytes.
     * @throws std::runtime_error when @p data is shorter than Header.
     */
    Message(std::span<const std::byte> data)
        : m_header(parse_header(data)),
          m_payload(data.subspan(sizeof(Header))) {}

    /**
     * @brief Attach a resolved topic name to this message view.
     *
     * @param topic_name Topic name. The caller must keep this string alive.
     */
    void set_topic(std::string_view topic_name) { m_topic = topic_name; }

    /**
     * @brief Return the resolved topic name when one has been attached.
     */
    std::string_view topic() const { return m_topic; }

    /**
     * @brief Return a copy of the parsed message header.
     */
    Header header() const { return *m_header; }

    /**
     * @brief Return the message payload bytes.
     */
    std::span<const std::byte> payload() const { return m_payload; }

    /**
     * @brief Interpret the payload bytes as a string view.
     *
     * The payload is not required to be null-terminated.
     */
    std::string_view payload_as_string() const {
        return std::string_view(reinterpret_cast<const char*>(m_payload.data()),
                                m_payload.size());
    }

    /**
     * @brief Render the message as a diagnostic string with hex payload bytes.
     *
     * @return Human-readable message description.
     */
    std::string to_string() const {
        static constexpr char hex[] = "0123456789abcdef";

        std::string out = "Message { " + header().to_string() + ", Payload { ";

        for (size_t i = 0; i < m_payload.size(); ++i) {
            if (i != 0) {
                out += ' ';
            }

            auto byte = std::to_integer<unsigned char>(m_payload[i]);

            out += "0x";
            out += hex[(byte >> 4) & 0x0f];
            out += hex[byte & 0x0f];
        }

        out += " } }";
        return out;
    }
};

} // namespace spraybus::networking::protocol
