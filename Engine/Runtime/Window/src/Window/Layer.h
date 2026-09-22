#pragma once
#include "Window/Event.h"
#include <Render/Render.h>
#include <ImGui/Core/imgui.h>
#include <memory>
#include <vector>
namespace Aether {
class Window;
class Layer
{
public:
    friend class Window;
public:
    virtual ~Layer() = default;
    Layer() = default;
    // 挂载的window负责调用OnAttach和OnDetach，并把自己传给Layer
    virtual void OnAttach(Window* window)
    {
    }
    virtual void OnDetach()
    {
    }
    virtual void OnUpdate(float sec)
    {
    }
    virtual void OnEvent(Event& e)
    {
    }

    virtual void OnFrameBegin()
    {
    }
    virtual void OnBuildRenderGraph(RenderGraph::RenderGraph& renderGraph)
    {

    }
    virtual bool NeedRebuildRenderGraph()
    {
        return false;
    }
    virtual void OnImGuiUpdate()
    {

    }
    virtual void OnUpload(PendingUploadList& pendingUploadList)
    {

    }
    // Main-thread extraction boundary. Implementations copy all state needed
    // by rendering into frame and never place Layer/World pointers in it.
    virtual void ExtractRenderData(Render::RenderFeatureFrame& frame)
    {
        (void)frame;
    }
    // Features contain render-owner state and outlive the frontend Layer when
    // retained by an accepted frame or an old RenderGraph.
    virtual void CollectRenderFeatures(std::vector<std::shared_ptr<Render::RenderFeature>>& features)
    {
        (void)features;
    }

};
} // namespace Aether
