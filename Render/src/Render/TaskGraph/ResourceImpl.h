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
using Texture = Resource<DeviceTexture, TextureDesc>;
using Buffer = Resource<DeviceBuffer, BufferDesc>;

struct FrameBufferDesc
{
    Texture* colorAttachments[DeviceRenderPassDesc::MaxColorAttachments] = {};
    Texture* depthAttachment = nullptr;
    size_t colorAttachmentCount = 0; // number of color attachments
    uint32_t width = 0;
    uint32_t height = 0;
    bool operator==(const FrameBufferDesc& other) const
    {
        if (colorAttachmentCount != other.colorAttachmentCount
            || depthAttachment != other.depthAttachment
            || width != other.width
            || height != other.height)
        {
            return false;
        }
        for (size_t i = 0; i < colorAttachmentCount; ++i)
        {
            if (colorAttachments[i] != other.colorAttachments[i])
            {
                return false;
            }
        }
        return true;
    }
};
template <>
struct Hash<FrameBufferDesc>
{
    size_t operator()(const FrameBufferDesc& desc) const
    {
        size_t hash = 0;
        for (size_t i = 0; i < desc.colorAttachmentCount; ++i)
        {
            hash ^= std::hash<void*>()(desc.colorAttachments[i]);
        }
        if (desc.depthAttachment)
        {
            hash ^= std::hash<void*>()(desc.depthAttachment);
        }
        hash ^= std::hash<uint32_t>()(desc.width) ^ std::hash<uint32_t>()(desc.height);
        hash ^= std::hash<size_t>()(desc.colorAttachmentCount);
        return hash;
    }
};
using FrameBuffer = Resource<DeviceFrameBuffer, FrameBufferDesc>;
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
template <>
struct GetResourceType<FrameBufferDesc>
{
    static constexpr ResourceType type = ResourceType::FrameBuffer;
};

struct Attachment
{
    Texture* texture = nullptr;
    DeviceAttachmentLoadOp loadOp;
    DeviceAttachmentStoreOp storeOp;
    bool operator==(const Attachment& other) const
    {
        return texture == other.texture && loadOp == other.loadOp && storeOp == other.storeOp;
    }
    bool operator!=(const Attachment& other) const
    {
        return !(other == *this);
    }
};
struct RenderPassDesc
{
    Attachment colorAttachment[DeviceRenderPassDesc::MaxColorAttachments];
    std::optional<Attachment> depthAttachment;
    size_t colorAttachmentCount = 0; // number of color attachments
    float clearColor[4]= {0.0f, 1.0f, 0.0f, 1.0f}; // default clear color
    bool operator==(const RenderPassDesc& other)
    {
        for (size_t i = 0; i < colorAttachmentCount; ++i)
        {
            if (colorAttachment[i] != other.colorAttachment[i])
            {
                return false;
            }
        }
        return depthAttachment == other.depthAttachment;
    }
};
template <>
struct Hash<Attachment>
{
    size_t operator()(const Attachment& attachment) const
    {
        return std::hash<void*>()(attachment.texture) ^ std::hash<uint32_t>()(static_cast<uint32_t>(attachment.loadOp)) ^ std::hash<uint32_t>()(static_cast<uint32_t>(attachment.storeOp));
    }
};
template <>
struct Hash<RenderPassDesc>
{
    size_t operator()(const RenderPassDesc& desc) const
    {
        size_t hash = 0;
        for (size_t i = 0; i < desc.colorAttachmentCount; ++i)
        {
            hash ^= Hash<Attachment>()(desc.colorAttachment[i]);
        }
        if (desc.depthAttachment)
        {
            hash ^= Hash<Attachment>()(*desc.depthAttachment);
        }
        return hash;
    }
};

} // namespace Aether::TaskGraph