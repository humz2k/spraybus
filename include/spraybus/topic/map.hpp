#pragma once

/**
 * @file map.hpp
 * @brief In-memory mapping from topic names to numeric topic keys.
 */

#include <cstdint>
#include <optional>
#include <string>
#include <unordered_map>

namespace spraybus::topic {

/**
 * @brief Assigns stable numeric keys to topic names for one server process.
 *
 * Keys are generated monotonically starting at 1 and are not persisted across
 * server restarts.
 */
class Map {
  private:
    std::unordered_map<std::string, uint64_t> m_map;

    uint64_t m_next_key = 1;
    uint64_t generate_key() { return m_next_key++; }

  public:
    /**
     * @brief Look up a topic key, creating one when the topic is new.
     *
     * @param topic Topic name.
     * @return Existing or newly generated topic key.
     */
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

    /**
     * @brief Look up an existing topic key.
     *
     * @param topic Topic name.
     * @return The topic key, or std::nullopt when the topic is unknown.
     */
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
