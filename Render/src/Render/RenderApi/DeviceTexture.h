#pragma once
#include "../Vulkan/Texture2D.h"
#include "Render/Config.h"
#include "Render/PixelFormat.h"
#include <cassert>
#include <variant>
#include <expected>
#include "../Config.h"
#include "Render/Vulkan/Buffer.h"
#include "Render/Vulkan/GlobalRenderContext.h"
#include "DeviceBuffer.h"
#include "Render/Vulkan/ImageView.h"
#include "vulkan/vulkan_core.h"
#include "DeviceImageView.h"
#include <Render/Id.h>
namespace Aether
{

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
    TransferSrc = Bit(1),
    TransferDst = Bit(2),
    ColorAttachment = Bit(3),
    DepthAttachment = Bit(4),
};
using DeviceImageUsageFlags = uint32_t;
inline VkImageUsageFlags DeviceImageUsageFlagsToVk(DeviceImageUsageFlags flags)
{
    VkImageUsageFlags res=0;
    if (flags & (uint32_t)DeviceImageUsage::Sample)
    {
        res |= VK_IMAGE_USAGE_SAMPLED_BIT;
    }
    if (flags & (uint32_t)DeviceImageUsage::TransferSrc)
    {
        res |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    }
    if (flags & (uint32_t)DeviceImageUsage::TransferDst)
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
inline DeviceImageUsageFlags VkImageUsageFlagsToRenderApi(VkImageUsageFlags flags)
{
    DeviceImageUsageFlags res = 0;
    if (flags & VK_IMAGE_USAGE_SAMPLED_BIT)
    {
        res |= (uint32_t)DeviceImageUsage::Sample;
    }
    if (flags & VK_IMAGE_USAGE_TRANSFER_SRC_BIT)
    {
        res |= (uint32_t)DeviceImageUsage::TransferSrc;
    }
    if (flags & VK_IMAGE_USAGE_TRANSFER_DST_BIT)
    {
        res |= (uint32_t)DeviceImageUsage::TransferDst;
    }
    if (flags & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT)
    {
        res |= (uint32_t)DeviceImageUsage::ColorAttachment;
    }
    if (flags & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)
    {
        res |= (uint32_t)DeviceImageUsage::DepthAttachment;
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
    DeviceTexture() =default;

    ~DeviceTexture()
    {
        if(m_Id)
        {
            Render::IdAllocator::Free(m_Id);
        }
    }
    DeviceTexture(DeviceTexture&& other)noexcept
    {
        m_Texture = std::move(other.m_Texture);
        m_DefaultImageView = std::move(other.m_DefaultImageView);
        m_Id = other.m_Id;
        other.m_Id = Render::Id<DeviceTexture>{};
    }
    DeviceTexture& operator=(DeviceTexture&& other)
    {
        if (this != &other)
        {
            if (m_Id)
            {
                Render::IdAllocator::Free(m_Id);
            }
            m_Texture = std::move(other.m_Texture);
            m_DefaultImageView = std::move(other.m_DefaultImageView);
            m_Id = other.m_Id;
            other.m_Id = Render::Id<DeviceTexture>{};
        }
        return *this;
    } 

    DeviceTexture(std::monostate) :
        m_Texture(std::monostate{})
    {
    }
public:
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
            auto res=DeviceTexture(std::move(texture.value()));
            res.m_Id = Render::IdAllocator::Allocate<DeviceTexture>();
            return res;
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
            auto res=DeviceTexture(std::move(texture.value()));
            res.m_Id = Render::IdAllocator::Allocate<DeviceTexture>();
            return res;
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
            auto res=DeviceTexture(std::move(texture.value()));
            res.m_Id = Render::IdAllocator::Allocate<DeviceTexture>();
            return res;
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
            auto res=DeviceTexture(std::move(texture.value()));
            res.m_Id = Render::IdAllocator::Allocate<DeviceTexture>();
            return res;
        }
        break;
        default:
            return std::unexpected<std::string>("Not implemented");
        }
    }
    static DeviceTexture Create(int width, int height, PixelFormat format, DeviceImageUsageFlags usages, DeviceImageLayout initLayout)
    {
        DeviceTexture res;
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
            res = std::move(texture.value());
            res.m_Id = Render::IdAllocator::Allocate<DeviceTexture>();
        }
        break;
        default:
            assert(false && "Not implemented");
            return std::monostate{};
        }
        return res;
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
        if (!m_DefaultImageView)
        {
            switch (Render::Config::RenderApi)
            {
            case Render::Api::Vulkan: {
                auto& texture = Get<vk::Texture2D>();
                auto imageView = vk::ImageView::Create(texture);
                assert(imageView && "Failed to create image view");
                m_DefaultImageView = CreateScope<DeviceImageView>(DeviceImageView(std::move(imageView.value())));
                return *m_DefaultImageView;
            }
            break;
            default:
                assert(false && "Not implemented");
                return *m_DefaultImageView;
            }
        }
        assert(m_DefaultImageView && "Default image view is not created");
        return *m_DefaultImageView;
    }
    DeviceImageView CreateImageView(const DeviceImageViewDesc& desc)
    {
        if (Render::Config::RenderApi == Render::Api::Vulkan)
        {
            auto& vkTexture = GetVk();
            auto imageView = vk::ImageView::Create(vkTexture);
            if (!imageView)
            {
                assert(false && "Failed to create image view");
                return DeviceImageView(std::monostate{});
            }
            return DeviceImageView(std::move(imageView.value()));
        }
        else
        {
            assert(false && "Not implemented");
            return DeviceImageView(std::monostate{});
        }
    }
    DeviceImageView& GetDefaultImageView()
    {
        return *m_DefaultImageView;
    }
    const DeviceImageView& GetDefaultImageView() const
    {
        return *m_DefaultImageView;
    }
    uint32_t GetWidth() const
    {
        return std::visit(GetWidthImpl{}, m_Texture);
    }
    uint32_t GetHeight() const
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
    DeviceImageUsageFlags GetUsages() const
    {
        switch (Render::Config::RenderApi)
        {
        case Render::Api::Vulkan: {
            auto& texture = Get<vk::Texture2D>();
            return VkImageUsageFlagsToRenderApi(texture.GetVkUsageFlags());
        }
        break;
        default:
            assert(false && "Not implemented");
            return 0;
        }
    }
    PixelFormat GetFormat() const
    {
        return std::visit(GetPixelFormatImpl{}, m_Texture);
    }
    Render::Id<DeviceTexture>& GetId()
    {
        return m_Id;
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
    struct GetPixelFormatImpl
    {
        PixelFormat operator()(const vk::Texture2D& texture) const
        {
            return texture.GetFormat();
        }
        PixelFormat operator()(std::monostate) const
        {
            assert(false&&"Empty DeviceTexture");
            return PixelFormat::R8;
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
    Scope<DeviceImageView> m_DefaultImageView;
    Render::Id<DeviceTexture> m_Id;
};
} // namespace Aether