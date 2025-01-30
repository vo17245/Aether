#pragma once
#include "../Vulkan/GraphicsCommandBuffer.h"
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

    template <typename T>
    DeviceCommandBufferView(T& t) :
        m_Data(&t)
    {
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

private:
    std::variant<std::monostate, vk::GraphicsCommandBuffer*> m_Data;
};
template <>
inline std::monostate& DeviceCommandBufferView::Get<std::monostate>()
{
    return std::get<std::monostate>(m_Data);
}
} // namespace Aether