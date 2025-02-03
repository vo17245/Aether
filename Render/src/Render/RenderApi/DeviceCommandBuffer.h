#pragma once
#include "../Vulkan/GraphicsCommandBuffer.h"
#include "Render/Vulkan/GraphicsCommandBuffer.h"
#include <cassert>
#include <variant>
namespace Aether
{
class DeviceCommandBuffer
{
public:
    DeviceCommandBuffer() = default;
    template <typename T>
    DeviceCommandBuffer(T&& t) :
        m_Data(std::forward<T>(t))
    {
    }
    template <typename T>
    T& Get()
    {
        return std::get<T>(m_Data);
    }
    const auto& GetData() const
    {
        return m_Data;
    }
    auto& GetData()
    {
        return m_Data;
    }
    bool Empty() const
    {
        return m_Data.index() == 0;
    }
    vk::GraphicsCommandBuffer& GetVk()
    {
        return std::get<vk::GraphicsCommandBuffer>(m_Data);
    }
    const vk::GraphicsCommandBuffer& GetVk()const
    {
        return std::get<vk::GraphicsCommandBuffer>(m_Data);
    }

private:
    std::variant<std::monostate, vk::GraphicsCommandBuffer> m_Data;
};
class DeviceCommandBufferView
{
public:
    DeviceCommandBufferView() = default;
    template <typename T>
    T& Get()
    {
        return *std::get<T*>(m_Data);
    }

    DeviceCommandBufferView(vk::GraphicsCommandBuffer& t) :
        m_Data(&t)
    {
    }
    DeviceCommandBufferView(DeviceCommandBuffer& t)
    {
        if (t.Empty())
        {
            m_Data = std::monostate();
        }
        else if(std::holds_alternative<vk::GraphicsCommandBuffer>(t.GetData()))
        {
            m_Data = &std::get<vk::GraphicsCommandBuffer>(t.GetData());
        }
        else
        {
            assert(false&&"unsupported command buffer type");
        }
    }
    DeviceCommandBufferView(std::monostate) :
        m_Data(std::monostate())
    {
    }
    bool Empty() const
    {
        return m_Data.index() == 0;
    }
    auto& GetData()
    {
        return m_Data;
    }
    vk::GraphicsCommandBuffer& GetVk()
    {
        return *std::get<vk::GraphicsCommandBuffer*>(m_Data);
    }
    const vk::GraphicsCommandBuffer& GetVk() const
    {
        return *std::get<vk::GraphicsCommandBuffer*>(m_Data);
    }

private:
    std::variant<std::monostate, vk::GraphicsCommandBuffer*> m_Data;
};
template <>
inline std::monostate& DeviceCommandBufferView::Get<std::monostate>()
{
    return std::get<std::monostate>(m_Data);
}
} // namespace Aether