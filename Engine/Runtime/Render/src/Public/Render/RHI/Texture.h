#pragma once
#include "Backend/Vulkan/Texture2D.h"
#include "Render/Config.h"
#include "Render/PixelFormat.h"
#include <cassert>
#include <variant>
#include <expected>
#include "../Config.h"
#include "Backend/Vulkan/Buffer.h"
#include <Render/Id.h>
namespace Aether::rhi
{

enum class TextureUsage : uint32_t
{
    Sample = Bit(0),
    TransferSrc = Bit(1),
    TransferDst = Bit(2),
    ColorAttachment = Bit(3),
    DepthAttachment = Bit(4),
};
using TextureUsageFlags = uint32_t;
inline VkImageUsageFlags RHITextureUsageFlagsToVk(TextureUsageFlags flags)
{
    VkImageUsageFlags res = 0;
    if (flags & (uint32_t)TextureUsage::Sample)
    {
        res |= VK_IMAGE_USAGE_SAMPLED_BIT;
    }
    if (flags & (uint32_t)TextureUsage::TransferSrc)
    {
        res |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    }
    if (flags & (uint32_t)TextureUsage::TransferDst)
    {
        res |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    }
    if (flags & (uint32_t)TextureUsage::ColorAttachment)
    {
        res |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    }
    if (flags & (uint32_t)TextureUsage::DepthAttachment)
    {
        res |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    }
    return res;
}
inline TextureUsageFlags VkImageUsageFlagsToRHITextureUsage(VkImageUsageFlags flags)
{
    TextureUsageFlags res = 0;
    if (flags & VK_IMAGE_USAGE_SAMPLED_BIT)
    {
        res |= (uint32_t)TextureUsage::Sample;
    }
    if (flags & VK_IMAGE_USAGE_TRANSFER_SRC_BIT)
    {
        res |= (uint32_t)TextureUsage::TransferSrc;
    }
    if (flags & VK_IMAGE_USAGE_TRANSFER_DST_BIT)
    {
        res |= (uint32_t)TextureUsage::TransferDst;
    }
    if (flags & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT)
    {
        res |= (uint32_t)TextureUsage::ColorAttachment;
    }
    if (flags & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)
    {
        res |= (uint32_t)TextureUsage::DepthAttachment;
    }
    return res;
}

class Texture
{
public:
    Texture() = default;

    ~Texture()
    {
       
    }
    Texture(Texture&& other) noexcept
    {
        m_Texture = std::move(other.m_Texture);
    }
    Texture& operator=(Texture&& other)
    {
        if (this != &other)
        {
            m_Texture = std::move(other.m_Texture);
        }
        return *this;
    }

    Texture(std::monostate) : m_Texture(std::monostate{})
    {
    }

public:
    bool Empty() const
    {
        return m_Texture.index() == 0;
    }

    Texture(vk::Texture2D&& t) : m_Texture(std::move(t))
    {
    }

    static Texture Create(int width, int height, PixelFormat format, TextureUsageFlags usages);

    uint32_t GetWidth() const
    {
        switch (Render::Config::RenderApi)
        {
        case Render::Api::Vulkan: {
            auto& texture = std::get<vk::Texture2D>(m_Texture);
            return texture.GetWidth();
        }
        break;
        default:
            assert(false && "Not implemented");
            return 0;
        }
    }
    uint32_t GetHeight() const
    {
        switch (Render::Config::RenderApi)
        {
        case Render::Api::Vulkan: {
            auto& texture = std::get<vk::Texture2D>(m_Texture);
            return texture.GetHeight();
        }
        break;
        default:
            assert(false && "Not implemented");
            return 0;
        }
    }

    operator bool() const
    {
        return !Empty();
    }

    TextureUsageFlags GetUsages() const
    {
        switch (Render::Config::RenderApi)
        {
        case Render::Api::Vulkan: {
            auto& texture = std::get<vk::Texture2D>(m_Texture);
            return VkImageUsageFlagsToRHITextureUsage(texture.GetVkUsageFlags());
        }
        break;
        default:
            assert(false && "Not implemented");
            return 0;
        }
    }
    PixelFormat GetFormat() const
    {
        switch (Render::Config::RenderApi)
        {
        case Render::Api::Vulkan: {
            auto& texture = std::get<vk::Texture2D>(m_Texture);
            return texture.GetFormat();
        }
        break;
        default:
            assert(false && "Not implemented");
            return PixelFormat::Unknown;
        }
    }

private:
    std::variant<std::monostate, vk::Texture2D> m_Texture;
};
} // namespace Aether::rhi