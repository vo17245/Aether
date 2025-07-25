#pragma once
#include <iostream>
#include <unordered_map>
#include <list>
#include <optional>
namespace Aether
{

template <typename Key, typename Value, typename Hash = std::hash<Key>>
class LRUCache
{
public:
    template <typename V>
    void Put(const Key& key, V&& value)
    {
        auto iter= m_CacheMap.find(key);
        if (iter != m_CacheMap.end())
        {
            m_CacheItems.erase(m_CacheMap[key]);
            m_CacheMap.erase(iter);
        }
        m_CacheItems.push_front(std::make_pair(key, std::forward<V>(value)));
        m_CacheMap[key] = m_CacheItems.begin();
    }
    void TrimToCapacity(size_t capacity)
    {
        while (m_CacheMap.size() > capacity)
        {
            auto last = m_CacheItems.end();
            --last;
            m_CacheMap.erase(last->first);
            m_CacheItems.pop_back();
        }
    }

    std::optional<Value> Get(const Key& key)
    {
        auto iter= m_CacheMap.find(key);
        if (iter == m_CacheMap.end())
        {
            return std::nullopt;
        }
        m_CacheItems.splice(m_CacheItems.begin(), m_CacheItems, iter->second);
        return iter->second->second;
    }
    std::optional<Value> Pop(const Key& key)
    {
        if (m_CacheMap.find(key) == m_CacheMap.end())
        {
            return std::nullopt;
        }
        auto it = m_CacheMap[key];
        Value value = std::move(it->second);
        m_CacheItems.erase(it);
        m_CacheMap.erase(key);
        return value;
    }
    void Erase(const Key& key)
    {
        if (m_CacheMap.find(key) != m_CacheMap.end())
        {
            auto it = m_CacheMap[key];
            m_CacheItems.erase(it);
            m_CacheMap.erase(key);
        }
    }
    bool Contains(const Key& key) const
    {
        return m_CacheMap.find(key) != m_CacheMap.end();
    }

private:
    std::list<std::pair<Key, Value>> m_CacheItems;
    std::unordered_map<Key, typename std::list<std::pair<Key, Value>>::iterator, Hash> m_CacheMap;
};
} // namespace Aether
