#pragma once
#include <Render/RenderApi.h>
#include "Resource.h"
#include "Handle.h"
#include "ResourceId.h"
#include "DeviceImageView.h"
namespace Aether::RenderGraph
{

struct FrameBufferDesc
{
    struct Attachment
    {
        ResourceId<DeviceImageView> imageView;
        DeviceAttachmentLoadOp loadOp = DeviceAttachmentLoadOp::Clear;
        DeviceAttachmentStoreOp storeOp = DeviceAttachmentStoreOp::Store;
    };

    constexpr static inline const uint32_t MaxColorAttachments = 8;
    Attachment colorAttachment;
    Attachment depthAttachment;
};

} // namespace Aether::RenderGraph