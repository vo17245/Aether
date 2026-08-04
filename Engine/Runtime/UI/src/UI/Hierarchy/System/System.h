#pragma once
#include <World/World.h>
#include <Window/Event.h>
#include <Render/RenderGraph/RenderGraph.h>

namespace Aether::UI
{
class SystemI
{
public:
    virtual void OnUpdate(float sec, World& scene) {}
    virtual void OnBuildRenderGraph(RenderGraph::RenderGraph& renderGraph,
                                    const RenderGraph::RenderPassDesc& renderPassDesc,
                                    Vec2f screenSize,
                                    World& scene)
    {
        (void)renderGraph;
        (void)renderPassDesc;
        (void)screenSize;
        (void)scene;
    }
    virtual void OnEvent(Event& event, World& scene) {}
    virtual ~SystemI() = default;
};
} // namespace Aether::UI
