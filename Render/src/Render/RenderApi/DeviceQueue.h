#pragma once
#include <Render/Vulkan/Queue.h>
namespace Aether
{
    class DeviceQueueView
    {
    public:
        DeviceQueueView(vk::Queue* queue) :
            m_Queue(std::move(queue))
        {
            assert(queue != nullptr);
        }
        DeviceQueueView() = default;
        DeviceQueueView(const DeviceQueueView&) = delete;
        DeviceQueueView(DeviceQueueView&& other)noexcept 
        {
            m_Queue = std::move(other.m_Queue);
            other.m_Queue = std::monostate{};
        }
        DeviceQueueView& operator=(const DeviceQueueView&) = delete;
        DeviceQueueView& operator=(DeviceQueueView&& other)noexcept 
        {
            m_Queue = std::move(other.m_Queue);
            other.m_Queue = std::monostate{};
            return *this;
        }
    public:
        vk::Queue* GetVk()const
        {
            assert(std::holds_alternative<vk::Queue*>(m_Queue));
            return std::get<vk::Queue*>(m_Queue);
        }
        
        operator bool()const
        {
            return !std::holds_alternative<std::monostate>(m_Queue);
        }
    private:
        std::variant<std::monostate,vk::Queue*> m_Queue;
    };
}