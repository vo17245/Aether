#pragma once
#include <functional>
#include <vector>
namespace Aether
{
    template<typename Ret,typename... Args>
    class Delegate
    {
    public:
        using CallbackType = std::function<Ret(Args...)>;
        uint32_t operator+=(CallbackType&& callback)
        {
            if(!callback)
                return UINT32_MAX;
            if(!m_FreeSlots.empty())
            {
                uint32_t index = m_FreeSlots.back();
                m_FreeSlots.pop_back();
                m_Callbacks[index]=std::move(callback);
                m_UsedSlots.push_back(index);
                return index;
            }
            m_Callbacks.push_back(std::move(callback));
            uint32_t index = static_cast<uint32_t>(m_Callbacks.size()-1);
            m_UsedSlots.push_back(index);
            return index;
        }
        void operator-=(uint32_t index)
        {
            auto it = std::find(m_UsedSlots.begin(),m_UsedSlots.end(),index);
            if(it!=m_UsedSlots.end())
            {
                m_Callbacks[index]=nullptr;
                m_UsedSlots.erase(it);
                m_FreeSlots.push_back(index);
            }
        }
        void Broadcast(Args... args)
        {
            for(auto index:m_UsedSlots)
            {
                if(m_Callbacks[index])
                    m_Callbacks[index](args...);
            }
        }
        
    private:
        std::vector<uint32_t> m_FreeSlots;
        std::vector<uint32_t> m_UsedSlots;
        std::vector<CallbackType> m_Callbacks;

    };
}