#pragma once
#include "Aether/Message/MessageBus.h"
#include "MessageBus.h"

namespace Aether
{
    class SubscribeReclaimer
    {
    public:
        SubscribeReclaimer()
        {
        }
        ~SubscribeReclaimer()
        {
            for(auto& cb:m_MsgCallbacks)
            {
                MessageBus::GetSingleton().Unsubscribe(cb);
            }
        }
        template<typename T>
        void Subscribe(const std::function<void(Message*)>& callback)
        {
            auto callbackSignature=MessageBus::GetSingleton().Subscribe<T>(callback);
            m_MsgCallbacks.emplace_back(callbackSignature);
        }
    private:
        std::vector<MessageBus::CallbackSignature> m_MsgCallbacks;
        
    };
}