#pragma once
#include <Render/RenderApi.h>
#include "DeviceTexture.h"
#include "ResourceId.h"
#include "AccessId.h"
namespace Aether::RenderGraph
{

struct Attachment
{
    AccessId<DeviceImageView> imageView;
    DeviceAttachmentLoadOp loadOp;
    DeviceAttachmentStoreOp storeOp;
    bool operator==(const Attachment& other) const
    {
        return imageView == other.imageView && loadOp == other.loadOp && storeOp == other.storeOp;
    }
    bool operator!=(const Attachment& other) const
    {
        return !(other == *this);
    }
};
}
