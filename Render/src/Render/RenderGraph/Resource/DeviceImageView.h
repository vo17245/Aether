#pragma once
#include <Render/RenderApi.h>
#include "Resource.h"
#include "AccessId.h"
#include <Core/Core.h>
namespace Aether::RenderGraph
{
    struct ImageViewDesc
    {
        AccessId<DeviceTexture> texture;
        DeviceImageViewDesc desc;
        bool operator==(const ImageViewDesc&) const = default;
    };
    template<>
    struct ResourceDescType<DeviceImageView>
    {
        using Type = ImageViewDesc;
    };

}
namespace Aether
{
    template<>
    struct Hash<RenderGraph::ImageViewDesc>
    {
        std::size_t operator()(const RenderGraph::ImageViewDesc& value) const
        {
            return Hash<RenderGraph::AccessId<DeviceTexture>>()(value.texture) ^
                   Hash<DeviceImageViewDesc>()(value.desc);
        }
    };
} // namespace Aether