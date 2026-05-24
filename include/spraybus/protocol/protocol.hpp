#pragma once

#include <spraybus/common/logging.hpp>

#include <cstddef>
#include <cstdint>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace spraybus::protocol {

enum class Origin : uint8_t {
    unknown = 0x00,
    client = 0x01,
    server = 0x02,
    leaf = 0x03,
};

std::string_view to_string(Origin origin) {
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

enum class Version : uint16_t {
    v1 = 0x01,
};

std::string_view to_string(Version version) {
    switch (version) {
    case Version::v1:
        return "v1";
    default:
        return "invalid";
    }
}

enum class Type : uint8_t {
    unknown = 0x00,
    topic_request = 0x01,
    topic_response = 0x02,
    publish = 0x03,
    subscribe = 0x04,
    unsubscribe = 0x05,
    fanout = 0x06,
};

std::string_view to_string(Type type) {
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

struct [[gnu::packed]] Header {
  public:
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
    static Header topic_request(Origin origin = Origin::client) {
        return Header(origin, Type::topic_request, 0);
    }

    static Header topic_response(uint64_t topic_key,
                                 Origin origin = Origin::server) {
        return Header(origin, Type::topic_response, topic_key);
    }

    static Header publish(uint64_t topic_key, Origin origin = Origin::client) {
        return Header(origin, Type::publish, topic_key);
    }

    static Header subscribe(uint64_t topic_key,
                            Origin origin = Origin::client) {
        return Header(origin, Type::subscribe, topic_key);
    }

    static Header unsubscribe(uint64_t topic_key,
                              Origin origin = Origin::client) {
        return Header(origin, Type::unsubscribe, topic_key);
    }

    static Header fanout(uint64_t topic_key, Origin origin = Origin::server) {
        return Header(origin, Type::fanout, topic_key);
    }

    Version version() const { return m_version_; }
    Origin origin() const { return m_origin_; }
    Type type() const { return m_type_; }
    uint64_t topic_key() const { return m_topic_key_; }

    std::string to_string() const {
        return "Header { version: " +
               std::string(protocol::to_string(m_version_)) +
               ", origin: " + std::string(protocol::to_string(m_origin_)) +
               ", type: " + std::string(protocol::to_string(m_type_)) +
               ", topic_key: " + std::to_string(m_topic_key_) + " }";
    }
};

static_assert(sizeof(Header) == 16, "Header must be exactly 16 bytes");

class Constructor {
  private:
    std::vector<std::byte> m_buffer;

  public:
    Constructor() = default;

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

    std::span<std::byte> topic_request(std::string_view topic_name,
                                       Origin origin = Origin::client) {
        Header header = Header::topic_request(origin);
        return construct(header, std::as_bytes(std::span<const char>(
                                     topic_name.data(), topic_name.size())));
    }

    std::span<std::byte> topic_response(uint64_t topic_key,
                                        std::string_view topic_name,
                                        Origin origin = Origin::server) {
        Header header = Header::topic_response(topic_key, origin);
        return construct(header, std::as_bytes(std::span<const char>(
                                     topic_name.data(), topic_name.size())));
    }

    std::span<std::byte> publish(uint64_t topic_key,
                                 std::span<const std::byte> payload,
                                 Origin origin = Origin::client) {
        Header header = Header::publish(topic_key, origin);
        return construct(header, payload);
    }

    std::span<std::byte> subscribe(uint64_t topic_key,
                                   Origin origin = Origin::client) {
        Header header = Header::subscribe(topic_key, origin);
        return construct(header);
    }

    std::span<std::byte> unsubscribe(uint64_t topic_key,
                                     Origin origin = Origin::client) {
        Header header = Header::unsubscribe(topic_key, origin);
        return construct(header);
    }

    std::span<std::byte> fanout(uint64_t topic_key,
                                std::span<const std::byte> payload,
                                Origin origin = Origin::server) {
        Header header = Header::fanout(topic_key, origin);
        return construct(header, payload);
    }
};

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
    Message(std::span<const std::byte> data)
        : m_header(parse_header(data)),
          m_payload(data.subspan(sizeof(Header))) {}

    void set_topic(std::string_view topic_name) { m_topic = topic_name; }

    std::string_view topic() const { return m_topic; }

    Header header() const { return *m_header; }
    std::span<const std::byte> payload() const { return m_payload; }
    std::string_view payload_as_string() const {
        return std::string_view(reinterpret_cast<const char*>(m_payload.data()),
                                m_payload.size());
    }

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

} // namespace spraybus::protocol

template <>
struct fmtquill::formatter<spraybus::protocol::Origin>
    : fmtquill::formatter<std::string_view> {
    auto format(const spraybus::protocol::Origin& value,
                format_context& ctx) const {
        const auto formatted = spraybus::protocol::to_string(value);
        return fmtquill::formatter<std::string_view>::format(formatted, ctx);
    }
};

template <>
struct quill::Codec<spraybus::protocol::Origin>
    : quill::DeferredFormatCodec<spraybus::protocol::Origin> {};

template <>
struct fmtquill::formatter<spraybus::protocol::Version>
    : fmtquill::formatter<std::string_view> {
    auto format(const spraybus::protocol::Version& value,
                format_context& ctx) const {
        const auto formatted = spraybus::protocol::to_string(value);
        return fmtquill::formatter<std::string_view>::format(formatted, ctx);
    }
};

template <>
struct quill::Codec<spraybus::protocol::Version>
    : quill::DeferredFormatCodec<spraybus::protocol::Version> {};

template <>
struct fmtquill::formatter<spraybus::protocol::Type>
    : fmtquill::formatter<std::string_view> {
    auto format(const spraybus::protocol::Type& value,
                format_context& ctx) const {
        const auto formatted = spraybus::protocol::to_string(value);
        return fmtquill::formatter<std::string_view>::format(formatted, ctx);
    }
};

template <>
struct quill::Codec<spraybus::protocol::Type>
    : quill::DeferredFormatCodec<spraybus::protocol::Type> {};

template <>
struct fmtquill::formatter<spraybus::protocol::Header>
    : fmtquill::formatter<std::string> {
    auto format(const spraybus::protocol::Header& value,
                format_context& ctx) const {
        const auto formatted = value.to_string();
        return fmtquill::formatter<std::string>::format(formatted, ctx);
    }
};

template <>
struct quill::Codec<spraybus::protocol::Header>
    : quill::DeferredFormatCodec<spraybus::protocol::Header> {};
