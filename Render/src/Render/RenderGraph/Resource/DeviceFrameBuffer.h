#pragma once
#include <Render/RenderApi.h>
#include "Resource.h"
#include "ResourceId.h"
#include "DeviceImageView.h"
#include "AccessId.h"
namespace Aether::RenderGraph
{

struct FrameBufferDesc
{
    struct Attachment
    {
        AccessId<DeviceImageView> imageView;
        DeviceAttachmentLoadOp loadOp = DeviceAttachmentLoadOp::Clear;
        DeviceAttachmentStoreOp storeOp = DeviceAttachmentStoreOp::Store;
    };
    ResourceId<DeviceRenderPass> renderPass;
    constexpr static inline const uint32_t MaxColorAttachments = 8;
    Attachment colorAttachment[MaxColorAttachments];
    uint32_t colorAttachmentCount = 0;
    std::optional<Attachment> depthAttachment;
    uint32_t width = 0;
    uint32_t height = 0;
};
template<>
struct ResourceDescType<DeviceFrameBuffer>
{
    using Type = FrameBufferDesc;
};

} // namespace Aether::RenderGraph