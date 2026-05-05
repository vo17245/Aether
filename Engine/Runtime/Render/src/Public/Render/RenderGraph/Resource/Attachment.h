#pragma once
#include <Render/RHI.h>
#include "ResourceId.h"
#include "AccessId.h"
namespace Aether::RenderGraph
{

struct Attachment
{
    AccessId<rhi::TextureView> textureView;
    rhi::AttachmentLoadOp loadOp;
    rhi::AttachmentStoreOp storeOp;
    bool operator==(const Attachment& other) const
    {
        return textureView == other.textureView && loadOp == other.loadOp && storeOp == other.storeOp;
    }
    bool operator!=(const Attachment& other) const
    {
        return !(other == *this);
    }
};
} // namespace Aether::RenderGraph
