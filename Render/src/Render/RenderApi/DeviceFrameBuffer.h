#pragma once
#include "../Vulkan/FrameBuffer.h"
#include <variant>
namespace Aether
{
class DeviceFrameBuffer
{
public:
    template <typename T>
    DeviceFrameBuffer(T&& t) :
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
    DeviceFrameBuffer()
    {
    }

private:
    std::variant<std::monostate, vk::FrameBuffer> m_Data;
};
class DeviceFrameBufferView
{
public:
    template <typename T>
    T& Get()
    {
        return *std::get<T*>(m_Data);
    }

    DeviceFrameBufferView()
    {
    }
    template <typename T>
    DeviceFrameBufferView(T& t) :
        m_Data(&t)
    {
    }
    DeviceFrameBufferView(std::monostate) :
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
    const auto& GetData() const
    {
        return m_Data;
    }

private:
    std::variant<std::monostate, vk::FrameBuffer*> m_Data;
};

template <>
inline std::monostate& DeviceFrameBufferView::Get<std::monostate>()
{
    return std::get<std::monostate>(m_Data);
}
} // namespace Aether