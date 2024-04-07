#pragma once
#include <functional>
#include <mutex>
#include <unordered_map>
#include <utility>
#include <vcruntime.h>
#include "IDGenerator.h"
#include "TypeIdProvider.h"
#include "Message.h"
#ifdef _WIN32
#undef DispatchMessage
#endif
namespace Aether
{


//thread safe message bus
class MessageBus
{
    friend class Application;
public:
    struct CallbackSignature
    {
        size_t topic_id;
        size_t callback_id;
        CallbackSignature(const CallbackSignature&) = default;
        CallbackSignature(size_t _topic_id, size_t _callback_id)
            :topic_id(_topic_id), callback_id(_callback_id) {}
    };
    static MessageBus& GetSingleton()
    {
        static MessageBus instance;
        return instance;
    } 
    template<typename T>
    CallbackSignature Subscribe(const std::function<void(Message*)>& callback)
    {
        std::lock_guard<std::mutex> lock(m_Mutex);
        size_t topicID = TypeIdProvider<T>::ID();
        size_t callback_id = m_IDGenerator.Get();
        m_Callbacks[topicID].emplace_back(callback_id, callback);
        return CallbackSignature(topicID,callback_id);
    }
    template<typename T>
    void Publish(T* data)
    {
        std::lock_guard<std::mutex> lock(m_Mutex);
        size_t topicID = TypeIdProvider<T>::ID();
        m_Messages.emplace_back(topicID, data);
    }
    bool Unsubscribe(CallbackSignature signature)
    {
        std::lock_guard<std::mutex> lock(m_Mutex);
        size_t topicID = signature.topic_id;
        auto& callbacks = m_Callbacks[topicID];
        for(auto it = callbacks.begin(); it != callbacks.end(); ++it)
        {
            if(it->first == signature.callback_id)
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
    std::unordered_map<size_t, std::vector<std::pair<size_t, std::function<void(Message*)>>>> m_Callbacks;
    std::vector<std::pair<size_t, Message*>> m_Messages;
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
            delete data;
        }
        m_Messages.clear();
    }
    std::mutex m_Mutex;
};
}
