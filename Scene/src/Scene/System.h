#pragma once
#include <Render/Render.h>
#include <Scene/Scene.h>
#include <Window/Event.h>
namespace Aether
{

class System
{
public:
    virtual void OnAttach(Scene* scene)
    {
    }
    virtual void OnUpdate(float deltaTime)
    {
    }
    virtual bool NeedRebuildRenderGraph()
    {
        return false;
    }
    virtual void OnBuildRenderGraph(RenderGraph::RenderGraph& renderGraph)
    {
    }
    virtual void OnUpload(PendingUploadList& uploadList)
    {
    }
    virtual ~System() = default;
    virtual void OnEvent(const Event& event)
    {
    }
    virtual void OnDetach()
    {
    }
};
} // namespace Aether
