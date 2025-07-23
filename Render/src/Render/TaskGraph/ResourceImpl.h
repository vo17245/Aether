#pragma once
#include "Resource.h"
#include <Render/RenderApi.h>
namespace Aether::TaskGraph
{
enum class BufferType : uint32_t
{
    SSBO,
    UBO,
    VBO,
    Staging
};
struct BufferDesc
{
    BufferType type;
    size_t size; // in bytes
    bool operator==(const BufferDesc& other) const
    {
        return type == other.type && size == other.size;
    }
};
template <>
struct Hash<BufferDesc>
{
    size_t operator()(const BufferDesc& desc)
    {
        return std::hash<uint32_t>()(static_cast<uint32_t>(desc.type)) ^ std::hash<size_t>()(desc.size);
    }
};

struct TextureDesc
{
    DeviceImageUsageFlags usages;
    PixelFormat pixelFormat;
    uint32_t width;
    uint32_t height;
    // The layout, when created in the task graph, 
    // will be changed in the task graph compilation phase (layout is dirty at that time !!!)
    // and does not affect the equality and hash functions. (aim to get texture from pool without check layout is same with the desc)
    DeviceImageLayout layout = DeviceImageLayout::Undefined; 
    bool operator==(const TextureDesc& other) const
    {
        return usages == other.usages && pixelFormat == other.pixelFormat && width == other.width && height == other.height;
    }
};
template <>
struct Hash<TextureDesc>
{
    size_t operator()(const TextureDesc& desc) const
    {
        return std::hash<uint32_t>()(static_cast<uint32_t>(desc.usages)) ^ std::hash<uint32_t>()(static_cast<uint32_t>(desc.pixelFormat)) ^ std::hash<uint32_t>()(desc.width) ^ std::hash<uint32_t>()(desc.height);
    }
};
using Texture=Resource<DeviceTexture, TextureDesc>;

using Buffer = Resource<DeviceBuffer, BufferDesc>;

template <>
struct GetResourceType<BufferDesc>
{
    static constexpr ResourceType type = ResourceType::Buffer;
};
template <>
struct GetResourceType<TextureDesc>
{
    static constexpr ResourceType type = ResourceType::Texture;
};

} // namespace Aether::TaskGraph