#pragma once
#include "../Vulkan/Texture2D.h"
#include "Render/Config.h"
#include "Render/PixelFormat.h"
#include "Render/RenderApi/DeviceCommandBuffer.h"
#include "Render/RenderApi/DeviceTexture.h"
#include <cassert>
#include <variant>
#include <expected>
#include "../Config.h"
#include "Render/Vulkan/Buffer.h"
#include "Render/Vulkan/GlobalRenderContext.h"
#include "DeviceBuffer.h"
#include "Render/Vulkan/ImageView.h"
#include "vulkan/vulkan_core.h"
namespace Aether
{
class DeviceImageView
{
public:
    bool Empty() const
    {
        return m_ImageView.index() == 0;
    }
    template <typename T>
    T& Get()
    {
        return std::get<T>(m_ImageView);
    }
    template <typename T>
    const T& Get() const
    {
        return std::get<T>(m_ImageView);
    }
    DeviceImageView(std::monostate) :
        m_ImageView(std::monostate{})
    {
    }
    DeviceImageView(vk::ImageView&& imageView) :
        m_ImageView(std::move(imageView))
    {
    }
    DeviceImageView()
    {
    }
    DeviceImageView(DeviceImageView&&) = default;
    DeviceImageView& operator=(DeviceImageView&&) = default;
    vk::ImageView& GetVk()
    {
        return std::get<vk::ImageView>(m_ImageView);
    }
    const vk::ImageView& GetVk() const
    {
        return std::get<vk::ImageView>(m_ImageView);
    }

private:
    std::variant<std::monostate, vk::ImageView> m_ImageView;
};
enum class DeviceImageLayout
{
    Present,
    ColorAttachment,
    Texture,
    Undefined,
    TransferDst,
    TransferSrc,
    DepthStencilAttachment,
};
enum class DeviceImageUsage : uint32_t
{
    Sample = Bit(0),
    Download = Bit(1),
    Upload = Bit(2),
    ColorAttachment = Bit(3),
    DepthAttachment = Bit(4),
};
using DeviceImageUsageFlags = uint32_t;
inline VkImageUsageFlags DeviceImageUsageFlagsToVk(DeviceImageUsageFlags flags)
{
    VkImageUsageFlags res;
    if (flags & (uint32_t)DeviceImageUsage::Sample)
    {
        res |= VK_IMAGE_USAGE_SAMPLED_BIT;
    }
    if (flags & (uint32_t)DeviceImageUsage::Download)
    {
        res |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    }
    if (flags & (uint32_t)DeviceImageUsage::Upload)
    {
        res |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    }
    if (flags & (uint32_t)DeviceImageUsage::ColorAttachment)
    {
        res |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    }
    if (flags & (uint32_t)DeviceImageUsage::DepthAttachment)
    {
        res |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    }
    return res;
}
inline VkImageLayout DeviceImageLayoutToVk(DeviceImageLayout layout)
{
    switch (layout)
    {
    case DeviceImageLayout::Present:
        return VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    case DeviceImageLayout::ColorAttachment:
        return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    case DeviceImageLayout::Texture:
        return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    case DeviceImageLayout::Undefined:
        return VK_IMAGE_LAYOUT_UNDEFINED;
    case DeviceImageLayout::TransferDst:
        return VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    case DeviceImageLayout::DepthStencilAttachment:
        return VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    case DeviceImageLayout::TransferSrc:
        return VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    default:
        assert(false && "Invalid layout");
        return VK_IMAGE_LAYOUT_UNDEFINED;
    }
}

class DeviceTexture
{
public:
    DeviceTexture() = default;
    bool Empty() const
    {
        return m_Texture.index() == 0;
    }
    template <typename T>
    T& Get()
    {
        return std::get<T>(m_Texture);
    }
    template <typename T>
    const T& Get() const
    {
        return std::get<T>(m_Texture);
    }
    DeviceTexture(vk::Texture2D&& t) :
        m_Texture(std::move(t))
    {
    }
    DeviceTexture(DeviceTexture&& t) = default;
    DeviceTexture& operator=(DeviceTexture&&) = default;
    DeviceTexture(std::monostate) :
        m_Texture(std::monostate{})
    {
    }
    /**
     * @deprecated This function is deprecated.Use Create Instead.
     * @note data should be in rgba_int8 format
     * usage: upload & sample
     */
    static std::expected<DeviceTexture, std::string> CreateForTexture(int width, int height, PixelFormat format)
    {
        switch (Render::Config::RenderApi)
        {
        case Render::Api::Vulkan: {
            auto texture = vk::Texture2D::CreateForTexture(width, height, format);
            if (!texture)
            {
                return std::unexpected<std::string>("Failed to create texture");
            }
            return DeviceTexture(std::move(texture.value()));
        }
        break;
        default:
            return std::unexpected<std::string>("Not implemented");
        }
    }
    /**
     * @deprecated This function is deprecated.Use Create Instead.
    */
    // usage:  download & upload & sample
    static std::optional<DeviceTexture> CreateForDownloadableTexture(int width, int height, PixelFormat format)
    {
        switch (Render::Config::RenderApi)
        {
        case Render::Api::Vulkan: {
            auto texture = vk::Texture2D::CreateForDownloadableTexture(width, height, format);
            if (!texture)
            {
                return std::nullopt;
            }
            return DeviceTexture(std::move(texture.value()));
        }
        break;
        default:
            return std::nullopt;
        }
    }
    /**
     * @deprecated This function is deprecated.Use Create Instead.
    */
    // usage: color attachment & sample
    static std::expected<DeviceTexture, std::string> CreateForColorAttachment(int width, int height, PixelFormat format)
    {
        switch (Render::Config::RenderApi)
        {
        case Render::Api::Vulkan: {
            auto texture = vk::Texture2D::CreateForColorAttachment(width, height, format);
            if (!texture)
            {
                return std::unexpected<std::string>("Failed to create texture");
            }
            return DeviceTexture(std::move(texture.value()));
        }
        break;
        default:
            return std::unexpected<std::string>("Not implemented");
        }
    }
    /**
     * @deprecated This function is deprecated.Use Create Instead.
    */
    // usage: depth attachment & sample
    static std::expected<DeviceTexture, std::string> CreateForDepthAttachment(int width, int height, PixelFormat format)
    {
        switch (Render::Config::RenderApi)
        {
        case Render::Api::Vulkan: {
            auto texture = vk::Texture2D::CreateForDepthAttachment(width, height, format);
            if (!texture)
            {
                return std::unexpected<std::string>("Failed to create texture");
            }
            return DeviceTexture(std::move(texture.value()));
        }
        break;
        default:
            return std::unexpected<std::string>("Not implemented");
        }
    }
    static DeviceTexture Create(int width, int height, PixelFormat format, DeviceImageUsageFlags usages, DeviceImageLayout initLayout)
    {
        switch (Render::Config::RenderApi)
        {
        case Render::Api::Vulkan: {
            auto vkUsages = DeviceImageUsageFlagsToVk(usages);
            auto vkInitLayout = DeviceImageLayoutToVk(initLayout);
            auto texture = vk::Texture2D::Create(width, height, format, vkUsages, vkInitLayout);
            if (!texture)
            {
                return std::monostate{};
            }
            return std::move(texture.value());
        }
        break;
        default:
            assert(false && "Not implemented");
            return std::monostate{};
        }
    }
    std::optional<std::string> CopyBuffer(const DeviceBuffer& buffer)
    {
        switch (Render::Config::RenderApi)
        {
        case Render::Api::Vulkan: {
            auto& vkBuffer = buffer.Get<vk::Buffer>();
            auto& texture = Get<vk::Texture2D>();
            texture.SyncCopyBuffer(vkBuffer);
            return std::nullopt;
        }
        break;
        default:
            return "Not implemented";
        }
    }
    DeviceImageView& GetOrCreateDefaultImageView()
    {
        if (m_DefaultImageView.Empty())
        {
            switch (Render::Config::RenderApi)
            {
            case Render::Api::Vulkan: {
                auto& texture = Get<vk::Texture2D>();
                auto imageView = vk::ImageView::Create(texture);
                if (!imageView)
                {
                    assert(false && "Failed to create image view");
                    return m_DefaultImageView;
                }
                m_DefaultImageView = DeviceImageView(std::move(imageView.value()));
            }
            break;
            default:
                assert(false && "Not implemented");
                return m_DefaultImageView;
            }
        }
        return m_DefaultImageView;
    }
    DeviceImageView& GetDefaultImageView()
    {
        return m_DefaultImageView;
    }
    const DeviceImageView& GetDefaultImageView() const
    {
        return m_DefaultImageView;
    }
    size_t GetWidth() const
    {
        return std::visit(GetWidthImpl{}, m_Texture);
    }
    size_t GetHeight() const
    {
        return std::visit(GetHeightImpl{}, m_Texture);
    }
    vk::Texture2D& GetVk()
    {
        return Get<vk::Texture2D>();
    }
    const vk::Texture2D& GetVk() const
    {
        return Get<vk::Texture2D>();
    }
    void AsyncTransitionLayout(DeviceImageLayout oldLayout, DeviceImageLayout newLayout, DeviceCommandBufferView commandBuffer)
    {
        std::visit(AsyncTransitionLayoutImpl{oldLayout, newLayout, commandBuffer}, m_Texture);
    }
    void SyncTransitionLayout(DeviceImageLayout oldLayout, DeviceImageLayout newLayout)
    {
        std::visit(SyncTransitionLayoutImpl{oldLayout, newLayout}, m_Texture);
    }
    operator bool() const
    {
        return !Empty();
    }
    void CopyToBuffer(const DeviceBuffer& buffer)
    {
        switch (Render::Config::RenderApi)
        {
        case Render::Api::Vulkan: {
            auto& vkBuffer = buffer.Get<vk::Buffer>();
            auto& texture = Get<vk::Texture2D>();
            texture.SyncCopyToBuffer(vkBuffer);
        }
        break;
        default:
            assert(false && "Not implemented");
        }
    }

private:
    struct GetWidthImpl
    {
        size_t operator()(const vk::Texture2D& texture) const
        {
            return texture.GetWidth();
        }
        size_t operator()(std::monostate) const
        {
            return 0;
        }
    };
    struct GetHeightImpl
    {
        size_t operator()(const vk::Texture2D& texture) const
        {
            return texture.GetHeight();
        }
        size_t operator()(std::monostate) const
        {
            return 0;
        }
    };
    struct AsyncTransitionLayoutImpl
    {
        DeviceImageLayout oldLayout;
        DeviceImageLayout newLayout;
        DeviceCommandBufferView commandBuffer;
        void operator()(vk::Texture2D& texture)
        {
            auto& cb = commandBuffer.GetVk();
            texture.AsyncTransitionLayout(cb, DeviceImageLayoutToVk(oldLayout), DeviceImageLayoutToVk(newLayout));
        }
        void operator()(std::monostate&)
        {
            assert(false && "texture is empty");
        }
    };
    struct SyncTransitionLayoutImpl
    {
        DeviceImageLayout oldLayout;
        DeviceImageLayout newLayout;
        void operator()(vk::Texture2D& texture)
        {
            texture.SyncTransitionLayout(DeviceImageLayoutToVk(oldLayout), DeviceImageLayoutToVk(newLayout));
        }
        void operator()(std::monostate&)
        {
            assert(false && "texture is empty");
        }
    };

    std::variant<std::monostate, vk::Texture2D> m_Texture;
    DeviceImageView m_DefaultImageView;
};
} // namespace Aether