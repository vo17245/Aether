#pragma once
#include <iostream>
#include <unordered_map>
#include <list>
#include <optional>
namespace Aether
{

template <typename Key, typename Value,typename Hash=std::hash<Key>>
class LRUCache {
public:
    LRUCache(size_t capacity) : m_Capacity(capacity) {}

    void Put(const Key& key, const Value& value) {
        if (m_CacheMap.find(key) != m_CacheMap.end()) {
            m_CacheItems.erase(m_CacheMap[key]);
        }
        m_CacheItems.push_front(std::make_pair(key, value));
        m_CacheMap[key] = m_CacheItems.begin();

        if (m_CacheMap.size() > m_Capacity) {
            auto last = m_CacheItems.end();
            --last;
            m_CacheMap.erase(last->first);
            m_CacheItems.pop_back();
        }
    }

    std::optional<Value> Get(const Key& key) {
        if (m_CacheMap.find(key) == m_CacheMap.end()) {
            return std::nullopt;
        }
        auto it = m_CacheMap[key];
        m_CacheItems.splice(m_CacheItems.begin(), m_CacheItems, it);
        return it->second;
    }
    void Erase(const Key& key) {
        if (m_CacheMap.find(key) != m_CacheMap.end()) {
            auto it = m_CacheMap[key];
            m_CacheItems.erase(it);
            m_CacheMap.erase(key);
        }
    }
    bool Contains(const Key& key) const {
        return m_CacheMap.find(key) != m_CacheMap.end();
    }

private:
    size_t m_Capacity;
    std::list<std::pair<Key, Value>> m_CacheItems;
    std::unordered_map<Key, typename std::list<std::pair<Key, Value>>::iterator,Hash> m_CacheMap;
};
}
