#pragma once
#include "../Vulkan/RenderPass.h"
#include "Render/Vulkan/RenderPass.h"
#include <variant>
#include <cassert>
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
    vk::RenderPass& GetVk()
    {
        return std::get<vk::RenderPass>(m_Data);
    }
    const vk::RenderPass& GetVk() const
    {
        return std::get<vk::RenderPass>(m_Data);
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
    template<typename T>
    const T& Get()const
    {
        return *std::get<T*>(m_Data);
    }

 
    DeviceRenderPassView(vk::RenderPass& t) :
        m_Data(&t)
    {
    }
    DeviceRenderPassView(DeviceRenderPass& t)
    {
        if (std::holds_alternative<std::monostate>(t.GetData()))
        {
            m_Data = std::monostate();
        }
        else if (std::holds_alternative<vk::RenderPass>(t.GetData()))
        {
            auto& renderPass = t.Get<vk::RenderPass>();
            m_Data = &renderPass;
        }
        else
        {
            assert(false && "unknown renderpass type");
        }
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
    vk::RenderPass& GetVk()
    {
        return *std::get<vk::RenderPass*>(m_Data);
    }
    const vk::RenderPass& GetVk() const
    {
        return *std::get<vk::RenderPass*>(m_Data);
    }

private:
    std::variant<std::monostate, vk::RenderPass*> m_Data; // do not own the object
};




} // namespace Aether