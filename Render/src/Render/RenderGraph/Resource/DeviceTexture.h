#pragma once
#include <Render/RenderApi.h>
#include "Resource.h"
#include <Core/Core.h>
namespace Aether::RenderGraph
{
struct TextureDesc
{
    DeviceImageUsageFlags usages;
    PixelFormat pixelFormat;
    uint32_t width;
    uint32_t height;

    DeviceImageLayout layout = DeviceImageLayout::Undefined;
    bool operator==(const TextureDesc& other) const
    {
        return usages == other.usages && pixelFormat == other.pixelFormat && width == other.width && height == other.height && layout == other.layout;
    }
};
template<>
struct ResourceDescType<DeviceTexture>
{
    using Type = TextureDesc;
};
template<>
struct Realize<DeviceTexture>
{
    Scope<DeviceTexture> operator()(const TextureDesc& desc)
    {
        auto deviceTexture = DeviceTexture::Create(desc.width, desc.height, desc.pixelFormat, desc.usages, desc.layout);
        if (!deviceTexture)
        {
            return nullptr;
        }
        return CreateScope<DeviceTexture>(std::move(deviceTexture));
    }
};

} // namespace Aether::RenderGraph
namespace Aether
{
    template<>
    struct Hash<RenderGraph::TextureDesc>
    {
        std::size_t operator()(const Aether::RenderGraph::TextureDesc& value) const
        {
            return std::hash<uint32_t>()(static_cast<uint32_t>(value.usages)) ^
                   std::hash<Aether::PixelFormat>()(value.pixelFormat) ^
                   std::hash<uint32_t>()(value.width) ^
                   std::hash<uint32_t>()(value.height) ^
                   std::hash<uint32_t>()(static_cast<uint32_t>(value.layout));
        }
    };
}