#pragma once
#include <Render/Render.h>
namespace Aether::URP
{
    class SceneRenderer
    {
    public:
        SceneRenderer() = default;
        ~SceneRenderer() = default;
        void RegisterRenderPasses(RenderGraph::RenderGraph& renderGraph);
    };
}