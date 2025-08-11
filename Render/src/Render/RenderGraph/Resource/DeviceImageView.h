#pragma once
#include <Render/RenderApi.h>
#include "Resource.h"
#include "AccessId.h"
namespace Aether::RenderGraph
{
    struct ImageViewDesc
    {
        AccessId<DeviceTexture> texture;
        DeviceImageViewDesc desc;
    };
    template<>
    struct ResourceDescType<DeviceImageView>
    {
        using Type = ImageViewDesc;
    };

}