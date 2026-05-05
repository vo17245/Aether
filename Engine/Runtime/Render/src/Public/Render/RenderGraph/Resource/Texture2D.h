#pragma once
#include "Resource.h"
#include <Core/Core.h>
#include <Render/RHI.h>
namespace Aether::RenderGraph
{
struct TextureDesc
{
    rhi::TextureUsageFlags usages;
    PixelFormat pixelFormat;
    uint32_t width;
    uint32_t height;

    rhi::TextureLayout layout = rhi::TextureLayout::Undefined;
    bool operator==(const TextureDesc& other) const
    {
        return usages == other.usages && pixelFormat == other.pixelFormat && width == other.width
               && height == other.height && layout == other.layout;
    }
};
template <>
struct ResourceDescType<rhi::Texture2D>
{
    using Type = TextureDesc;
};
template <>
struct Realize<rhi::Texture2D>
{
    Scope<rhi::Texture2D> operator()(const TextureDesc& desc)
    {
        auto textureDesc = rhi::TextureDesc{.usages = desc.usages,
                                            .pixelFormat = desc.pixelFormat,
                                            .width = desc.width,
                                            .height = desc.height,
                                            .layout = desc.layout};
        auto deviceTexture = rhi::Texture2D::Create(textureDesc);
        deviceTexture.SyncTransitionLayout(rhi::TextureLayout::Undefined, desc.layout);
        if (!deviceTexture)
        {
            return nullptr;
        }
        return CreateScope<rhi::Texture2D>(std::move(deviceTexture));
    }
};

} // namespace Aether::RenderGraph
namespace Aether
{
template <>
struct Hash<RenderGraph::TextureDesc>
{
    std::size_t operator()(const Aether::RenderGraph::TextureDesc& value) const
    {
        return std::hash<uint32_t>()(static_cast<uint32_t>(value.usages))
               ^ std::hash<Aether::PixelFormat>()(value.pixelFormat) ^ std::hash<uint32_t>()(value.width)
               ^ std::hash<uint32_t>()(value.height) ^ std::hash<uint32_t>()(static_cast<uint32_t>(value.layout));
    }
};
} // namespace Aether