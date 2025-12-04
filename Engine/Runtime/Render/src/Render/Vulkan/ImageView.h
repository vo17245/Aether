#pragma once

#include "vulkan/vulkan_core.h"
#include <cstdint>
#include <optional>
#include "Texture2D.h"
namespace Aether {
namespace vk {

class ImageView
{
public:
    static std::optional<ImageView> Create(const Texture2D& texture);
    static std::optional<ImageView> CreateForDepthAttachment(Texture2D& texture);
    // create for swapchain image
    static std::optional<ImageView> Create(VkImage image, VkFormat format, uint32_t width, uint32_t height);
    ~ImageView();
    VkImageView GetHandle() const;
    ImageView(const ImageView&) = delete;
    ImageView& operator=(const ImageView&) = delete;
    ImageView(ImageView&& other) noexcept;
    ImageView& operator=(ImageView&& other) noexcept;
    VkFormat GetFormat() const;
    uint32_t GetWidth() const;
    uint32_t GetHeight() const;

private:
    ImageView(VkImageView imageView, VkFormat format, uint32_t width, uint32_t height);
    VkImageView m_ImageView;
    VkFormat m_Format;
    uint32_t m_Width;
    uint32_t m_Height;
};
}
} // namespace Aether::vk