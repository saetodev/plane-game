#ifndef MAP_H
#define MAP_H

#include "Basic.h"

#include <memory>

template<typename K, typename V>
struct MapEntry {
    K first;
    V second;

    bool empty = true;
};

template<typename K, typename V, size_t Capacity>
class Map {
public:
    void Add(const K& key, const V& value) {
        ASSERT(!Full());
        size_t index = std::hash<K>{}(key) % Capacity;

        while (!m_data[index].empty) {
            index = (index + 1) % Capacity;
        }

        m_data[index] = { key, value, false };
        m_size += 1;
    }

    void Remove(const K& key) {
        ASSERT(Contains(key));

        size_t index = std::hash<K>{}(key) % Capacity;

        while (true) {
            MapEntry<K, V>& entry = m_data[index];

            if (entry.empty) {
                index = (index + 1) % Capacity;
                continue;
            }

            if (key == entry.first) {
                entry.empty = true;
                m_size -= 1;

                return;
            }

            index = (index + 1) % Capacity;
        }
    }

    V& Get(const K& key) {
        ASSERT(Contains(key));

        size_t index = std::hash<K>{}(key) % Capacity;

        while (true) {
            MapEntry<K, V>& entry = m_data[index];

            if (entry.empty) {
                index = (index + 1) % Capacity;
                continue;
            }

            if (key == entry.first) {
                return entry.second;
            }

            index = (index + 1) % Capacity;
        }

        ASSERT(false);
    }

    void Set(const K& key, const V& value) {
        ASSERT(Contains(key));

        size_t index = std::hash<K>{}(key) % Capacity;

        while (true) {
            MapEntry<K, V>& entry = m_data[index];

            if (entry.empty) {
                index = (index + 1) % Capacity;
                continue;
            }

            if (key == entry.first) {
                entry.second = value;
                return;
            }

            index = (index + 1) % Capacity;
        }
    }

    bool Contains(const K& key) const {
        if (Empty()) {
            return false;
        }

        size_t index = std::hash<K>{}(key) % Capacity;
        size_t currentIndex = index;

        while (true) {
            if (m_data[currentIndex].empty) {
                goto NEXT_ENTRY;
            }

            if (key == m_data[currentIndex].first) {
                return true;
            }

NEXT_ENTRY:
            currentIndex = (currentIndex + 1) % Capacity;

            // key not in map
            if (index == currentIndex) {
                return false;
            }
        }

        return false;
    }

    bool Full() const {
        return m_size == Capacity;
    }

    bool Empty() const {
        return m_size == 0;
    }

private:
    size_t m_size = 0;
    MapEntry<K, V> m_data[Capacity] = {};
};

#endif