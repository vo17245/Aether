#pragma once
#include "../Vulkan/FrameBuffer.h"
#include <cassert>
#include <type_traits>
#include <variant>
#include "DeviceTexture.h"
#include "Render/Config.h"
#include "DeviceRenderPass.h"
#include "Render/Vulkan/ImageView.h"
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
    static DeviceFrameBuffer CreateFromTexture(const DeviceRenderPassView& renderPass,DeviceTexture& texture)
    {
        if (Render::Config::RenderApi == Render::Api::Vulkan)
        {
            auto& vkTexture=texture.Get<vk::Texture2D>();
            auto& vkRenderPass=renderPass.Get<vk::RenderPass>();
            VkExtent2D size={vkTexture.GetWidth(),vkTexture.GetHeight()};
            auto& imageView=texture.GetOrCreateDefaultImageView().Get<vk::ImageView>();
            auto frameBuffer=vk::FrameBuffer::Create(vkRenderPass, size,imageView);
            return DeviceFrameBuffer(std::move(frameBuffer.value()));
        }
        else
        {
            assert(false && "not implemented");
            return {};
        }
    }
    static DeviceFrameBuffer CreateFromColorAttachmentAndDepthAttachment(const DeviceRenderPassView& renderPass,
    DeviceTexture& color,DeviceTexture& depth)
    {
        if (Render::Config::RenderApi == Render::Api::Vulkan)
        {
            auto& vkColor=color.Get<vk::Texture2D>();
            auto& vkRenderPass=renderPass.Get<vk::RenderPass>();
            VkExtent2D size={vkColor.GetWidth(),vkColor.GetHeight()};
            auto& colorImageView=color.GetOrCreateDefaultImageView().Get<vk::ImageView>();
            auto& vkDepth=depth.Get<vk::Texture2D>();
            auto& depthImageView=depth.GetOrCreateDefaultImageView().Get<vk::ImageView>();

            auto frameBuffer=vk::FrameBuffer::Create(vkRenderPass,
             size,colorImageView,depthImageView);
            return DeviceFrameBuffer(std::move(frameBuffer.value()));
        }
        else
        {
            assert(false && "not implemented");
            return {};
        }
    }
    vk::FrameBuffer& GetVk()
    {
        return std::get<vk::FrameBuffer>(m_Data);
    }
    const vk::FrameBuffer& GetVk() const
    {
        return std::get<vk::FrameBuffer>(m_Data);
    }
    Vec2u GetSize()const
    {
        if (std::holds_alternative<vk::FrameBuffer>(m_Data))
        {
            auto& frameBuffer = std::get<vk::FrameBuffer>(m_Data);
            return Vec2u(frameBuffer.GetSize().width, frameBuffer.GetSize().height);
        }
        else
        {
            assert(false && "not implemented");
            return {0,0};
        }
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
    DeviceFrameBufferView(vk::FrameBuffer& t) :
        m_Data(&t)
    {
    }
    DeviceFrameBufferView(DeviceFrameBuffer& t)
    {
        if (std::holds_alternative<vk::FrameBuffer>(t.GetData()))
        {
            auto& frameBuffer = t.Get<vk::FrameBuffer>();
            m_Data = &frameBuffer;
        }
        else if (std::holds_alternative<std::monostate>(t.GetData()))
        {
            m_Data = std::monostate();
        }
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
    vk::FrameBuffer& GetVk()
    {
        return *std::get<vk::FrameBuffer*>(m_Data);
    }
    const vk::FrameBuffer& GetVk() const
    {
        return *std::get<vk::FrameBuffer*>(m_Data);
    }
    Vec2u GetSize()const
    {
        if (std::holds_alternative<vk::FrameBuffer*>(m_Data))
        {
            auto& frameBuffer = *std::get<vk::FrameBuffer*>(m_Data);
            return Vec2u(frameBuffer.GetSize().width, frameBuffer.GetSize().height);
        }
        else
        {
            assert(false && "not implemented");
            return {0,0};
        }
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