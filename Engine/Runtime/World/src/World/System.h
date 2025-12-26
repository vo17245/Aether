#pragma once
#include <Render/Render.h>
#include <World/World.h>
#include <Window/Event.h>
namespace Aether
{

class System
{
public:
    virtual std::string_view GetSignature()const=0;
    virtual std::vector<std::string_view> GetDependencies()const=0;
    virtual void OnAttach(World* scene)
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
    virtual void OnEvent(Event& event)
    {
    }
    virtual void OnDetach()
    {
    }
};
} // namespace Aether
