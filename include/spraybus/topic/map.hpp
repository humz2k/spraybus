#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <unordered_map>

namespace spraybus::topic {

class Map {
  private:
    std::unordered_map<std::string, uint64_t> m_map;

    uint64_t m_next_key = 1;
    uint64_t generate_key() { return m_next_key++; }

  public:
    uint64_t get_or_create(const std::string& topic) {
        auto it = m_map.find(topic);
        if (it != m_map.end()) {
            return it->second;
        } else {
            uint64_t topic_key = generate_key();
            m_map[topic] = topic_key;
            return topic_key;
        }
    }

    std::optional<uint64_t> get(const std::string& topic) const {
        auto it = m_map.find(topic);
        if (it != m_map.end()) {
            return it->second;
        } else {
            return std::nullopt;
        }
    }
};

} // namespace spraybus::topic
