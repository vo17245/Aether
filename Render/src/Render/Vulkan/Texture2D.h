#pragma once
#include <cstdint>
#include <optional>
#include "Buffer.h"
#include "Render/PixelFormat.h"
#include "vulkan/vulkan_core.h"
namespace Aether {
namespace vk {

class Texture2D
{
public:
    static std::optional<Texture2D> Create(uint32_t width, uint32_t height,
                                           PixelFormat format, VkImageUsageFlags usage, VkImageLayout initialLayout = VK_IMAGE_LAYOUT_UNDEFINED);

    static std::optional<Texture2D> CreateForTexture(uint32_t width, uint32_t height, PixelFormat format);
    static std::optional<Texture2D> CreateForColorAttachment(uint32_t width, uint32_t height, PixelFormat format);
    static std::optional<Texture2D> CreateForDepthAttachment(uint32_t width, uint32_t height, PixelFormat format);
    ~Texture2D();

    operator bool();
    Texture2D(const Texture2D&) = delete;
    Texture2D& operator=(const Texture2D&) = delete;
    Texture2D(Texture2D&& other) noexcept;
    Texture2D& operator=(Texture2D&& other) noexcept;

    void SyncTransitionLayout(GraphicsCommandPool& commandPool, VkImageLayout oldLayout, VkImageLayout newLayout);
    void SyncCopyBuffer(GraphicsCommandPool& commandPool, Buffer& buffer);
    VkImage GetHandle() const;
    PixelFormat GetFormat() const;
    uint32_t GetWidth() const;
    uint32_t GetHeight() const;
    VkFormat GetVkFormat() const;

private:
    Texture2D(VkImage image, VmaAllocation allocation, VmaAllocationInfo allocInfo, PixelFormat format, uint32_t width, uint32_t height);
    Texture2D();

private:
    VkImage m_Image = nullptr;
    VmaAllocation m_Allocation = nullptr;
    VmaAllocationInfo m_AllocInfo;
    PixelFormat m_Format;
    uint32_t m_Width;
    uint32_t m_Height;
    VkFormat m_VkFormat;

public:
    enum class Usage
    {
        Texture,
        ColorAttachment,
        DepthAttachment,
        Unknown,
    };

    Usage GetUsage()
    {
        return m_Usage;
    }

private:
    // 用于在生成imageview 时填 VkImageViewCreateInfo.subresourceRange.aspectMask
    // 如果是DepthAttachment填VK_IMAGE_ASPECT_DEPTH_BIT
    // 其他 的填VK_IMAGE_ASPECT_COLOR_BIT
    Usage m_Usage = Usage::Unknown;
};
}
} // namespace Aether::vk