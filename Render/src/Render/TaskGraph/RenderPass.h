#pragma once
#include "Resource.h"
#include <Render/RenderApi.h>
#include "ResourceImpl.h"
namespace Aether::TaskGraph
{
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
    size_t colorAttachmentCount = 0;             // number of color attachments
    Vec4f clearColor = {0.0f, 1.0f, 0.0f, 1.0f}; // default clear color
    bool operator==(const RenderPassDesc& other) const
    {
        if (colorAttachmentCount != other.colorAttachmentCount)
        {
            return false;
        }
        if (clearColor != other.clearColor)
        {
            return false;
        }
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