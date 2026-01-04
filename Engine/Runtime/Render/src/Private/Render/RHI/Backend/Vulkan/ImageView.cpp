#include "Render/RHI/Backend/Vulkan/ImageView.h"
#include "Render/RHI/Backend/Vulkan/GlobalRenderContext.h"
namespace Aether {
namespace vk {
std::optional<ImageView> ImageView::Create(const Texture2D& texture)
{
    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = texture.GetHandle();
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = texture.GetVkFormat();
    viewInfo.subresourceRange.aspectMask =
        texture.GetVkUsageFlags()&VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT ?
            VK_IMAGE_ASPECT_DEPTH_BIT :
            VK_IMAGE_ASPECT_COLOR_BIT;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    VkImageView imageView;
    if (vkCreateImageView(GRC::GetDevice(), &viewInfo, nullptr, &imageView) != VK_SUCCESS)
    {
        return std::nullopt;
    }
    return ImageView(imageView, texture.GetVkFormat(), texture.GetWidth(), texture.GetHeight());
}
std::optional<ImageView> ImageView::CreateForDepthAttachment(Texture2D& texture)
{
    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = texture.GetHandle();
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = texture.GetVkFormat();
    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    VkImageView imageView;
    if (vkCreateImageView(GRC::GetDevice(), &viewInfo, nullptr, &imageView) != VK_SUCCESS)
    {
        return std::nullopt;
    }
    return ImageView(imageView, texture.GetVkFormat(), texture.GetWidth(), texture.GetHeight());
}
// create for swapchain image
std::optional<ImageView> ImageView::Create(VkImage image, VkFormat format, uint32_t width, uint32_t height)
{
    VkImageView imageView;
    VkImageViewCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    createInfo.image = image;
    createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    createInfo.format = format;
    createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    createInfo.subresourceRange.baseMipLevel = 0;
    createInfo.subresourceRange.levelCount = 1;
    createInfo.subresourceRange.baseArrayLayer = 0;
    createInfo.subresourceRange.layerCount = 1;

    if (vkCreateImageView(GRC::GetDevice(), &createInfo, nullptr, &imageView) != VK_SUCCESS)
    {
        return std::nullopt;
    }
    return ImageView(imageView, format, width, height);
}
ImageView::~ImageView()
{
    if (m_ImageView != VK_NULL_HANDLE)
        vkDestroyImageView(GRC::GetDevice(), m_ImageView, nullptr);
}
VkImageView ImageView::GetHandle() const
{
    return m_ImageView;
}
ImageView::ImageView(ImageView&& other) noexcept
{
    m_ImageView = other.m_ImageView;
    other.m_ImageView = VK_NULL_HANDLE;
    m_Width = other.m_Width;
    m_Height = other.m_Height;
}
ImageView& ImageView::operator=(ImageView&& other) noexcept
{
    if (this != &other)
    {
        if (m_ImageView != VK_NULL_HANDLE)
            vkDestroyImageView(GRC::GetDevice(), m_ImageView, nullptr);
        m_ImageView = other.m_ImageView;
        other.m_ImageView = VK_NULL_HANDLE;
        m_Width = other.m_Width;
        m_Height = other.m_Height;
    }
    return *this;
}
VkFormat ImageView::GetFormat() const
{
    return m_Format;
}
uint32_t ImageView::GetWidth() const
{
    return m_Width;
}
uint32_t ImageView::GetHeight() const
{
    return m_Height;
}

ImageView::ImageView(VkImageView imageView, VkFormat format, uint32_t width, uint32_t height) :
    m_ImageView(imageView), m_Format(format), m_Width(width), m_Height(height)
{
}
}
} // namespace Aether::vk