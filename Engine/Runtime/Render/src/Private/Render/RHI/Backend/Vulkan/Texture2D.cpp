#include "Texture2D.h"
#include "GraphicsCommandBuffer.h"
#include "GlobalRenderContext.h"
#include "Render/PixelFormat.h"
#include "Allocator.h"
#include "vulkan/vulkan_core.h"
#include <Core/Core.h>
namespace Aether::vk
{

VkFormat Texture2D::GetVkFormat() const
{
    return PixelFormatToVkFormat(m_Format);
}
std::optional<Texture2D> Texture2D::Create(uint32_t width,
                                           uint32_t height,
                                           PixelFormat format,
                                           VkImageUsageFlags usage,
                                           VkImageLayout initialLayout)
{
    VkImage textureImage = nullptr;
    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = static_cast<uint32_t>(width);
    imageInfo.extent.height = static_cast<uint32_t>(height);
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;

    imageInfo.format = PixelFormatToVkFormat(format);
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.initialLayout = initialLayout;
    imageInfo.usage = usage; // VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.flags = 0; // Optional

    VmaAllocationCreateInfo allocCreateInfo = {};
    allocCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;
    // allocCreateInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_ALLOW_TRANSFER_INSTEAD_BIT | VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT;
    VmaAllocation allocation = nullptr;

    VmaAllocationInfo allocInfo;
    auto res = vmaCreateImage(Allocator::Get(),
                              &imageInfo,
                              &allocCreateInfo,
                              &textureImage,
                              &allocation,
                              &allocInfo);
    if (res != VK_SUCCESS)
    {
        if (textureImage != VK_NULL_HANDLE)
        {
            vkDestroyImage(GRC::GetDevice(), textureImage, nullptr);
        }
        if (allocation != nullptr)
        {
            vmaFreeMemory(Allocator::Get(), allocation);
        }
        return std::nullopt;
    }

    auto texture = Texture2D(textureImage, allocation, allocInfo, format, width, height);
    texture.m_VkUsageFlags = usage;
    return texture;
}
std::optional<Texture2D> Texture2D::CreateForTexture(uint32_t width, uint32_t height, PixelFormat format)
{
    auto res = Create(width, height, format, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
    // auto res = Create(width, height, format, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
    res->m_Usage = Usage::Texture;
    return res;
}
std::optional<Texture2D> Texture2D::CreateForDownloadableTexture(uint32_t width, uint32_t height, PixelFormat format)
{
    auto res = Create(width, height, format, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
    res->m_Usage = Usage::Texture;
    return res;
}
std::optional<Texture2D> Texture2D::CreateForColorAttachment(uint32_t width, uint32_t height, PixelFormat format)
{
    auto res = Create(width, height, format, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
    res->m_Usage = Usage::ColorAttachment;
    return res;
}
Texture2D::~Texture2D()
{
    if (m_Image != VK_NULL_HANDLE)
    {
        vmaDestroyImage(Allocator::Get(), m_Image, m_Allocation);
    }
}
Texture2D::operator bool()
{
    return m_Image != VK_NULL_HANDLE;
}
Texture2D::Texture2D(Texture2D&& other) noexcept
{
    m_Image = other.m_Image;
    m_Allocation = other.m_Allocation;
    m_AllocInfo = other.m_AllocInfo;
    m_Usage = other.m_Usage;
    other.m_Image = VK_NULL_HANDLE;
    other.m_Allocation = VK_NULL_HANDLE;
    m_Width = other.m_Width;
    m_Height = other.m_Height;
    m_Format = other.m_Format;
    m_VkUsageFlags=other.m_VkUsageFlags;
}
Texture2D& Texture2D::operator=(Texture2D&& other) noexcept
{
    if (this != &other)
    {
        if (m_Image != VK_NULL_HANDLE)
        {
            vmaDestroyImage(Allocator::Get(), m_Image, m_Allocation);
        }
        m_Image = other.m_Image;
        m_Allocation = other.m_Allocation;
        m_AllocInfo = other.m_AllocInfo;
        m_Usage = other.m_Usage;
        other.m_Image = VK_NULL_HANDLE;
        other.m_Allocation = VK_NULL_HANDLE;
        m_Width = other.m_Width;
        m_Height = other.m_Height;
        m_Format = other.m_Format;
        m_VkUsageFlags=other.m_VkUsageFlags;
    }
    return *this;
}
static void SetTransitionLayoutParam(VkImageMemoryBarrier& barrier,
                                     VkPipelineStageFlags& sourceStage,
                                     VkPipelineStageFlags& destinationStage,
                                     VkImageLayout oldLayout, VkImageLayout newLayout,
                                     VkImage image,
                                     VkImageAspectFlags aspectMask)
{
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;
    barrier.subresourceRange.aspectMask = aspectMask;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;
    barrier.srcAccessMask = 0; // TODO
    barrier.dstAccessMask = 0; // TODO

    if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
    {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    }
    else if ((oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) && (newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL))
    {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }
    else if ((oldLayout) == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL && (newLayout) == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
    {
        barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
        barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        sourceStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        destinationStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    }
    else if ((oldLayout) == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL && (newLayout) == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
    {
        barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        sourceStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }
    else if ((oldLayout == VK_IMAGE_LAYOUT_UNDEFINED) && (newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL))
    {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }
    else if ((oldLayout == VK_IMAGE_LAYOUT_UNDEFINED) && (newLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL))
    {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    }
    else if ((oldLayout == VK_IMAGE_LAYOUT_UNDEFINED) && (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL))
    {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    }
    else if ((oldLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) && (newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL))
    {
        barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        sourceStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    }
    else if ((oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) && (newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL))
    {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }
    else
    {
        assert(false && "unsupported layout transition!");
    }
}

void Texture2D::SyncTransitionLayout(GraphicsCommandPool& commandPool, VkImageLayout oldLayout, VkImageLayout newLayout)
{
    auto fenceOpt = Fence::Create();
    auto fence = std::move(fenceOpt.value());

    auto cbopt = GraphicsCommandBuffer::Create(commandPool);
    GraphicsCommandBuffer cb = std::move(cbopt.value());
    cb.BeginSingleTime();
    VkImageMemoryBarrier barrier{};
    VkPipelineStageFlags sourceStage = 0;
    VkPipelineStageFlags destinationStage = 0;
    VkImageAspectFlags aspectMask;
    if (m_Format == PixelFormat::R_FLOAT32_DEPTH)
    {
        aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    }
    else
    {
        aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    }
    SetTransitionLayoutParam(barrier, sourceStage, destinationStage, oldLayout, newLayout, m_Image, aspectMask);
    vkCmdPipelineBarrier(
        cb.GetHandle(),
        sourceStage, destinationStage,
        0,
        0, nullptr,
        0, nullptr,
        1, &barrier);
    cb.End();
    cb.BeginSubmit().Fence(fence).EndSubmit();
    fence.Wait();
}
void Texture2D::SyncCopyBuffer(GraphicsCommandPool& commandPool, const Buffer& buffer)
{
    auto fenceOpt = Fence::Create();
    auto fence = std::move(fenceOpt.value());
    auto cbopt = GraphicsCommandBuffer::Create(commandPool);
    GraphicsCommandBuffer cb = std::move(cbopt.value());
    VkBufferImageCopy region{};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;

    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;

    region.imageOffset = {0, 0, 0};
    region.imageExtent = {
        m_Width,
        m_Height,
        1};
    cb.BeginSingleTime();
    vkCmdCopyBufferToImage(
        cb.GetHandle(),
        buffer.GetHandle(),
        m_Image,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        1,
        &region);
    cb.End();
    cb.BeginSubmit().Fence(fence).EndSubmit();
    fence.Wait();
}
VkImage Texture2D::GetHandle() const
{
    return m_Image;
}
void Texture2D::SyncCopyBuffer(const Buffer& buffer)
{
    SyncCopyBuffer(GRC::GetGraphicsCommandPool(), buffer);
}
PixelFormat Texture2D::GetFormat() const
{
    return m_Format;
}
uint32_t Texture2D::GetWidth() const
{
    return m_Width;
}
uint32_t Texture2D::GetHeight() const
{
    return m_Height;
}

Texture2D::Texture2D(VkImage image, VmaAllocation allocation, VmaAllocationInfo allocInfo, PixelFormat format, uint32_t width, uint32_t height) :
    m_Image(image), m_Allocation(allocation), m_AllocInfo(allocInfo), m_Format(format), m_Width(width), m_Height(height)
{
}
Texture2D::Texture2D()
{
    memset(&m_AllocInfo, 0, sizeof(VmaAllocationInfo));
}
std::optional<Texture2D> Texture2D::CreateForDepthAttachment(uint32_t width, uint32_t height, PixelFormat format)
{
    auto res = Create(width, height, format, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
    res->m_Usage = Usage::DepthAttachment;
    return res;
}
void Texture2D::SyncTransitionLayout(VkImageLayout oldLayout, VkImageLayout newLayout)
{
    SyncTransitionLayout(GRC::GetGraphicsCommandPool(), oldLayout, newLayout);
}

void Texture2D::AsyncTransitionLayout(GraphicsCommandBuffer& cb, VkImageLayout oldLayout, VkImageLayout newLayout)
{
    VkImageMemoryBarrier barrier{};

    VkPipelineStageFlags sourceStage = 0;
    VkPipelineStageFlags destinationStage = 0;
    VkImageAspectFlags aspectMask;
    if (m_Format == PixelFormat::R_FLOAT32_DEPTH)
    {
        aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    }
    else
    {
        aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    }
    SetTransitionLayoutParam(barrier, sourceStage, destinationStage, oldLayout, newLayout, m_Image, aspectMask);
    vkCmdPipelineBarrier(
        cb.GetHandle(),
        sourceStage, destinationStage,
        0,
        0, nullptr,
        0, nullptr,
        1, &barrier);
}
void Texture2D::AsyncCopyTexture(GraphicsCommandBuffer& commandBuffer, const Texture2D& texture)
{
    assert(false && "not implement");
}
bool Texture2D::SyncCopyToBuffer(const Buffer& buffer)
{
    auto fenceOpt = Fence::Create();
    auto fence = std::move(fenceOpt.value());
    auto cbopt = GraphicsCommandBuffer::Create(GRC::GetGraphicsCommandPool());
    GraphicsCommandBuffer cb = std::move(cbopt.value());
    VkBufferImageCopy region{};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;

    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;

    region.imageOffset = {0, 0, 0};
    region.imageExtent = {
        m_Width,
        m_Height,
        1};
    cb.BeginSingleTime();
    vkCmdCopyImageToBuffer(
        cb.GetHandle(),
        m_Image,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        buffer.GetHandle(),
        1,
        &region);
    cb.End();
    cb.BeginSubmit().Fence(fence).EndSubmit();
    fence.Wait();
    return true;
}
} // namespace Aether::vk