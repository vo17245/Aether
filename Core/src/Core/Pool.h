#pragma once
#include <vector>
#include <unordered_map>
#include <list>
#include <optional>
namespace Aether
{
template <typename KeyType, typename ValueType, typename Hash = std::hash<KeyType>>
class LruPool
{
public:
    LruPool(size_t capacity) :
        m_Capacity(capacity)
    {
    }
    template<typename V>
    void Push(const KeyType& key, V&& value)
    {
        auto iter = m_DataMap.find(key);
        if (iter == m_DataMap.end())
        {
            iter = m_DataMap.insert({key, {}}).first;
        }
        m_Datas.push_front({key, std::forward<V>(value)});
        iter->second.push_back(m_Datas.begin());
        TrimToCapacity();
    }
    std::optional<ValueType> Pop(const KeyType& key)
    {
        auto iter = m_DataMap.find(key);
        if (iter == m_DataMap.end())
        {
            return std::nullopt;
        }
        if (iter->second.empty())
        {
            return std::nullopt;
        }
        auto it = iter->second.back();
        iter->second.pop_back();
        ValueType value = std::move(it->value);
        m_Datas.erase(it);
        if (iter->second.empty())
        {
            m_DataMap.erase(iter);
        }
        return value;
    }
    void SetCapacity(size_t capacity)
    {
        m_Capacity = capacity;
        TrimToCapacity();
    }
    size_t GetCapacity() const
    {
        return m_Capacity;
    }

private:
    void TrimToCapacity()
    {
        while (m_Datas.size() > m_Capacity)
        {
            auto last = m_Datas.end();
            --last;
            auto iter = m_DataMap.find(last->key);
            assert(iter != m_DataMap.end() && "LRU pool data map not match data list");
            for (auto it = iter->second.begin(); it != iter->second.end(); ++it)
            {
                if (*it == last)
                {
                    iter->second.erase(it);
                    break;
                }
            }
            m_Datas.pop_back();
        }
    }
    struct Entry
    {
        KeyType key;
        ValueType value;
    };
    std::list<Entry> m_Datas;
    std::unordered_map<KeyType, std::vector<typename std::list<Entry>::iterator>, Hash> m_DataMap;
    size_t m_Capacity;
};
} // namespace Aether