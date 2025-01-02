#include "FrameBuffer.h"

#include "GlobalRenderContext.h"
#include "vulkan/vulkan_core.h"
namespace Aether {
namespace vk {
std::optional<FrameBuffer> FrameBuffer::Create(const VkFramebufferCreateInfo& info)
{
    VkFramebuffer framebuffer;
    if (vkCreateFramebuffer(GRC::GetDevice(), &info, nullptr, &framebuffer) != VK_SUCCESS)
    {
        return std::nullopt;
    }
    return FrameBuffer(framebuffer, VkExtent2D{info.width, info.height});
}
FrameBuffer& FrameBuffer::operator=(FrameBuffer&& other) noexcept
{
    if (this != &other)
    {
        if (m_FrameBuffer != VK_NULL_HANDLE)
            vkDestroyFramebuffer(GRC::GetDevice(), m_FrameBuffer, nullptr);
        m_FrameBuffer = other.m_FrameBuffer;
        other.m_FrameBuffer = VK_NULL_HANDLE;
        m_Size = other.m_Size;
    }
    return *this;
}
FrameBuffer::~FrameBuffer()
{
    if (m_FrameBuffer != VK_NULL_HANDLE)
        vkDestroyFramebuffer(GRC::GetDevice(), m_FrameBuffer, nullptr);
}
}
} // namespace Aether::vk