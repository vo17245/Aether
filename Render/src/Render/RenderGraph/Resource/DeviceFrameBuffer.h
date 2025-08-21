#pragma once
#include <Render/RenderApi.h>
#include "Resource.h"
#include "ResourceId.h"
#include "DeviceImageView.h"
#include "AccessId.h"
#include "Attachment.h"
namespace Aether::RenderGraph
{

struct FrameBufferDesc
{
    
    AccessId<DeviceRenderPass> renderPass;
    constexpr static inline const uint32_t MaxColorAttachments = 8;
    Attachment colorAttachments[MaxColorAttachments];
    uint32_t colorAttachmentCount = 0;
    std::optional<Attachment> depthAttachment;
    uint32_t width = 0;
    uint32_t height = 0;
    bool operator==(const FrameBufferDesc& other) const
    {
        if (colorAttachmentCount != other.colorAttachmentCount || width != other.width || height != other.height)
        {
            return false;
        }
        for (uint32_t i = 0; i < colorAttachmentCount; ++i)
        {
            if (colorAttachments[i] != other.colorAttachments[i])
            {
                return false;
            }
        }
        if (depthAttachment.has_value() != other.depthAttachment.has_value())
        {
            return false;
        }
        if (depthAttachment && *depthAttachment != *other.depthAttachment)
        {
            return false;
        }
        return renderPass == other.renderPass;
    }
};
template <>
struct ResourceDescType<DeviceFrameBuffer>
{
    using Type = FrameBufferDesc;
};

} // namespace Aether::RenderGraph
namespace Aether
{
template <>
struct Hash<RenderGraph::FrameBufferDesc>
{
    std::size_t operator()(const RenderGraph::FrameBufferDesc& value) const
    {
        size_t hash = 0;
        for (size_t i = 0; i < value.colorAttachmentCount; ++i)
        {
            hash ^= Hash<RenderGraph::AccessId<DeviceImageView>>()(value.colorAttachments[i].imageView);
            hash ^= static_cast<size_t>(value.colorAttachments[i].loadOp);
            hash ^= static_cast<size_t>(value.colorAttachments[i].storeOp);
        }
        if (value.depthAttachment)
        {
            hash ^= Hash<RenderGraph::AccessId<DeviceImageView>>()(value.depthAttachment->imageView);
            hash ^= static_cast<size_t>(value.depthAttachment->loadOp);
            hash ^= static_cast<size_t>(value.depthAttachment->storeOp);
        }
        hash ^= std::hash<uint32_t>()(value.width);
        hash ^= std::hash<uint32_t>()(value.height);
        return hash;
    }
};
} // namespace Aether