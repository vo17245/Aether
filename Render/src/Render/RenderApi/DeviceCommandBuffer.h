#pragma once
#include "../Vulkan/GraphicsCommandBuffer.h"
#include "Render/Vulkan/GraphicsCommandBuffer.h"
#include <cassert>
#include <variant>
#include "DevicePipeline.h"
#include "DeviceDescriptorPool.h"
namespace Aether
{
class DeviceCommandBuffer
{
public:
    DeviceCommandBuffer() = default;
    template <typename T>
    DeviceCommandBuffer(T&& t) : m_Data(std::forward<T>(t))
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
    const vk::GraphicsCommandBuffer& GetVk() const
    {
        return std::get<vk::GraphicsCommandBuffer>(m_Data);
    }
    void BindDescriptorSet(DeviceDescriptorSet& set, DevicePipelineLayout& layout)
    {
        if (std::holds_alternative<vk::GraphicsCommandBuffer>(m_Data))
        {
            auto& cb = std::get<vk::GraphicsCommandBuffer>(m_Data);
            auto& vkSet = set.GetVk();
            auto& vkLayout = layout.GetVk();
            for (size_t i = 0; i < vkSet.sets.size(); i++)
            {
                cb.BindDescriptorSet(vkSet.sets[i], vkLayout, i);
            }
        }
        else
        {
            assert(false && "unsupported command buffer type");
        }
    }
    void ImageLayoutTransition(DeviceTexture& image, DeviceImageLayout oldLayout, DeviceImageLayout newLayout)
    {
        if (std::holds_alternative<vk::GraphicsCommandBuffer>(m_Data))
        {
            auto& cb = GetVk();
            image.GetVk().AsyncTransitionLayout(cb, DeviceImageLayoutToVk(oldLayout), DeviceImageLayoutToVk(newLayout));
        }
        else
        {
            assert(false && "unsupported command buffer type");
        }
    }
    void SetViewport(float x, float y, float width, float height)
    {
        if (std::holds_alternative<vk::GraphicsCommandBuffer>(m_Data)) 
        {
            auto& cb=GetVk();
            cb.SetViewport(x, y, width, height);
        }
        else
        {
            assert(false && "unsupported command buffer type");
        }
    }
    void SetScissor(float x, float y, float width, float height)
    {
        if (std::holds_alternative<vk::GraphicsCommandBuffer>(m_Data)) 
        {
            auto& cb=GetVk();
            cb.SetScissor(x, y, width, height);
        }
        else
        {
            assert(false && "unsupported command buffer type");
        }
    }
    void BindPipeline(DevicePipeline& pipeline)
    {
        if (std::holds_alternative<vk::GraphicsCommandBuffer>(m_Data)) 
        {
            auto& cb=GetVk();
            cb.BindPipeline(pipeline.GetVk());
        }
        else
        {
            assert(false && "unsupported command buffer type");
        }
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

    DeviceCommandBufferView(vk::GraphicsCommandBuffer& t) : m_Data(&t)
    {
    }
    DeviceCommandBufferView(DeviceCommandBuffer& t)
    {
        if (t.Empty())
        {
            m_Data = std::monostate();
        }
        else if (std::holds_alternative<vk::GraphicsCommandBuffer>(t.GetData()))
        {
            m_Data = &std::get<vk::GraphicsCommandBuffer>(t.GetData());
        }
        else
        {
            assert(false && "unsupported command buffer type");
        }
    }
    DeviceCommandBufferView(std::monostate) : m_Data(std::monostate())
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