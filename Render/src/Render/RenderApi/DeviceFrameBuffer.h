#pragma once
#include <cassert>
#include <variant>
#include "Render/Config.h"
#include "DeviceRenderPass.h"
#include <Render/RenderApi.h>
// for alloca
#ifdef _MSC_VER
#include <malloc.h>
#else
#include <alloca.h>
#endif
namespace Aether
{
struct DeviceFrameBufferDesc
{
    DeviceImageView* colorAttachments[DeviceRenderPassDesc::MaxColorAttachments];
    DeviceImageView* depthAttachment = nullptr;
    uint32_t width = 0;
    uint32_t height = 0;
    size_t colorAttachmentCount = 0; // number of color attachments
};
class DeviceFrameBuffer
{
public:
    template <typename T>
    DeviceFrameBuffer(T&& t) : m_Data(std::forward<T>(t))
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
    static DeviceFrameBuffer Create(const DeviceRenderPass& renderPass, const DeviceFrameBufferDesc& desc)
    {
        return Create(desc, &renderPass);
    }
    static DeviceFrameBuffer Create(const DeviceFrameBufferDesc& desc, const DeviceRenderPass* renderPass = nullptr)
    {
        if (Render::Config::RenderApi == Render::Api::Vulkan)
        {
            VkFramebufferCreateInfo frameBufferInfo{};
            frameBufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            if (renderPass)
            {
                frameBufferInfo.renderPass = renderPass->GetVk().GetHandle();
            }
            frameBufferInfo.width = desc.width;
            frameBufferInfo.height = desc.height;
            frameBufferInfo.layers = 1;
            size_t attachmentCount = 0;
            attachmentCount += desc.colorAttachmentCount;
            if (desc.depthAttachment)
            {
                attachmentCount++;
            }
            VkImageView* attachments = (VkImageView*)alloca(sizeof(VkImageView) * attachmentCount);
            for (size_t i = 0; i < desc.colorAttachmentCount; ++i)
            {
                attachments[i] = desc.colorAttachments[i]->GetVk().GetHandle();
            }
            if (desc.depthAttachment)
            {
                attachments[desc.colorAttachmentCount] = desc.depthAttachment->GetVk().GetHandle();
            }
            frameBufferInfo.attachmentCount = static_cast<uint32_t>(attachmentCount);
            frameBufferInfo.pAttachments = attachments;
            auto frameBuffer = vk::FrameBuffer::Create(frameBufferInfo);
            if (!frameBuffer.has_value())
            {
                return {};
            }
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
    Vec2u GetSize() const
    {
        if (std::holds_alternative<vk::FrameBuffer>(m_Data))
        {
            auto& frameBuffer = std::get<vk::FrameBuffer>(m_Data);
            return Vec2u(frameBuffer.GetSize().width, frameBuffer.GetSize().height);
        }
        else
        {
            assert(false && "not implemented");
            return {0, 0};
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
    DeviceFrameBufferView(vk::FrameBuffer& t) : m_Data(&t)
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
    DeviceFrameBufferView(std::monostate) : m_Data(std::monostate())
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
    Vec2u GetSize() const
    {
        if (std::holds_alternative<vk::FrameBuffer*>(m_Data))
        {
            auto& frameBuffer = *std::get<vk::FrameBuffer*>(m_Data);
            return Vec2u(frameBuffer.GetSize().width, frameBuffer.GetSize().height);
        }
        else
        {
            assert(false && "not implemented");
            return {0, 0};
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