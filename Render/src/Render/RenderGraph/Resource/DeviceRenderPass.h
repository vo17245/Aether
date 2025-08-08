#pragma once
#include <Render/RenderApi.h>
#include "Resource.h"
namespace Aether::RenderGraph
{
    template<>
    struct ResourceDescType<DeviceRenderPass>
    {
        using Type = DeviceRenderPassDesc;
    };
    template<>
    struct Realize<DeviceRenderPass>
    {
        Scope<DeviceRenderPass> operator()(const DeviceRenderPassDesc& desc)
        {
            return CreateScope<DeviceRenderPass>(DeviceRenderPass::Create(desc));
        }
    };
}