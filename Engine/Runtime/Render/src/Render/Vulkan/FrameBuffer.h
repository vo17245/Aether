#pragma once
#include <array>
#include <optional>
#include <tuple>
#include "Core/Base.h"
#include "Traits.h"
#include "vulkan/vulkan_core.h"
#include "RenderPass.h"
namespace Aether {
namespace vk {

class FrameBuffer
{
public:
    template <typename... Attachments>
        requires AreAllImageView<Attachments...>
    static std::optional<FrameBuffer> Create(const RenderPass& renderPass, const VkExtent2D& size, const Attachments&... attachments)
    {
        auto attachmentsArray = make_image_view_handle_array(attachments...);
        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = renderPass.GetHandle();
        framebufferInfo.attachmentCount = static_cast<uint32_t>(attachmentsArray.size());
        framebufferInfo.pAttachments = attachmentsArray.data();
        framebufferInfo.width = size.width;
        framebufferInfo.height = size.height;
        framebufferInfo.layers = 1;

        return Create(framebufferInfo);
    }

    template <typename... Attachments>
        requires AreAllImageView<Attachments...>
    static Scope<FrameBuffer> CreateScope(const RenderPass& renderPass, VkExtent2D size, const Attachments&... attachments)
    {
        auto opt = Create(renderPass, size, attachments...);
        if (!opt.has_value())
        {
            return nullptr;
        }
        return ::Aether::CreateScope<FrameBuffer>(std::move(opt.value()));
    }

    ~FrameBuffer();
    VkFramebuffer GetHandle() const
    {
        return m_FrameBuffer;
    }
    FrameBuffer(const FrameBuffer&) = delete;
    FrameBuffer& operator=(const FrameBuffer&) = delete;
    FrameBuffer(FrameBuffer&& other) noexcept
    {
        m_FrameBuffer = other.m_FrameBuffer;
        other.m_FrameBuffer = VK_NULL_HANDLE;
        m_Size = other.m_Size;
    }
    FrameBuffer& operator=(FrameBuffer&& other) noexcept;
    VkExtent2D GetSize() const
    {
        return m_Size;
    }
    static std::optional<FrameBuffer> Create(const VkFramebufferCreateInfo& info);
private:
    
    FrameBuffer(VkFramebuffer framebuffer, VkExtent2D size) :
        m_FrameBuffer(framebuffer), m_Size(size)
    {
    }

    VkFramebuffer m_FrameBuffer;
    VkExtent2D m_Size;
};
}
} // namespace Aether::vk