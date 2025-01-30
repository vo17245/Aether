#pragma once
#include "../Vulkan/RenderPass.h"
#include <variant>
namespace Aether
{
class DeviceRenderPass
{
public:
    DeviceRenderPass() = default;
    template <typename T>
    DeviceRenderPass(T&& t) :
        m_Data(std::forward<T>(t))
    {
    }
    DeviceRenderPass(std::monostate) :
        m_Data(std::monostate())
    {
    }
    bool Empty() const
    {
        return m_Data.index() == 0;
    }
    template <typename T>
    T& Get()
    {
        return std::get<T>(m_Data);
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
    std::variant<std::monostate, vk::RenderPass> m_Data;
};
class DeviceRenderPassView
{
public:
    DeviceRenderPassView() = default;
    template <typename T>
    T& Get()
    {
        return *std::get<T*>(m_Data);
    }

    template <typename T>
    DeviceRenderPassView(T& t) :
        m_Data(&t)
    {
    }
    DeviceRenderPassView(std::monostate) :
        m_Data(std::monostate())
    {
    }
    DeviceRenderPassView(const DeviceRenderPassView& other) = default;
    DeviceRenderPassView(DeviceRenderPassView&& other) = default;
    DeviceRenderPassView& operator=(const DeviceRenderPassView& other) = default;
    DeviceRenderPassView& operator=(DeviceRenderPassView&& other) = default;
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
    std::variant<std::monostate, vk::RenderPass*> m_Data; // do not own the object
};
template <>
inline DeviceRenderPassView::DeviceRenderPassView(DeviceRenderPassView& t)
{
    m_Data = t.m_Data;
} 
template <>
inline std::monostate& DeviceRenderPassView::Get<std::monostate>()
{
    return std::get<std::monostate>(m_Data);
}
struct DeviceRenderPassToView
{
    template <typename T>
    DeviceRenderPassView operator()(T& t)
    {
        return &t;
    }
};
template <>
inline DeviceRenderPassView DeviceRenderPassToView::operator()(std::monostate& t)
{
    return std::monostate();
}

} // namespace Aether