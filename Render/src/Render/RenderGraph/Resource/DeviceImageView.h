#pragma once
#include <Render/RenderApi.h>
#include "Resource.h"
#include "ResourceId.h"
namespace Aether::RenderGraph
{
    struct ImageViewDesc
    {
        ResourceId<DeviceTexture> texture;
    };
}