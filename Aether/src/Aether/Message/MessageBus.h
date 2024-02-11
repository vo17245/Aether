#pragma once
#include <functional>
#include <mutex>
#include <unordered_map>
#include <utility>
#include <vcruntime.h>
#include "IDGenerator.h"
#include "TypeIdProvider.h"
namespace Aether
{

//thread safe message bus
class MessageBus
{
    friend class Application;
public:
    static MessageBus& GetSingleton()
    {
        static MessageBus instance;
        return instance;
    } 
    template<typename T>
    size_t Subscribe(const std::function<void(void*)>& callback)
    {
        std::lock_guard<std::mutex> lock(m_Mutex);
        size_t topicID = TypeIdProvider<T>::ID();
        size_t callback_id = m_IDGenerator.Get();
        m_Callbacks[topicID].emplace_back(callback_id, callback);
        return callback_id;
    }
    template<typename T>
    size_t Publish(T* data)
    {
        std::lock_guard<std::mutex> lock(m_Mutex);
        size_t topicID = TypeIdProvider<T>::ID();
        m_Messages.emplace_back(topicID, data);
    }
    template<typename T>
    bool Unsubscribe(size_t callback_id)
    {
        std::lock_guard<std::mutex> lock(m_Mutex);
        size_t topicID = TypeIdProvider<T>::ID();
        auto& callbacks = m_Callbacks[topicID];
        for(auto it = callbacks.begin(); it != callbacks.end(); ++it)
        {
            if(it->first == callback_id)
            {
                callbacks.erase(it);
                return true;
            }
        }
        return false;
    }
    
private:
    MessageBus() = default;
    IDGenerator m_IDGenerator;
    std::unordered_map<size_t, std::vector<std::pair<size_t, std::function<void(void*)>>>> m_Callbacks;
    std::vector<std::pair<size_t, void*>> m_Messages;
    void DispatchMessage()
    {
        std::lock_guard<std::mutex> lock(m_Mutex);
        for(auto [topic_id,data] : m_Messages)
        {
            auto& callbacks = m_Callbacks[topic_id];
            for(auto& callback : callbacks)
            {
                callback.second(data);
            }
        }
        m_Messages.clear();
    }
    std::mutex m_Mutex;
};
}
