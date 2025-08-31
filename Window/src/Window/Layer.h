#pragma once
#include "Window/Event.h"
#include <Render/RenderApi.h>
#include <Render/RenderGraph/RenderGraph.h>
#include <ImGui/Core/imgui.h>
namespace Aether {
class Window;
class Layer
{
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
    virtual void RegisterRenderPasses(RenderGraph::RenderGraph& renderGraph)
    {

    }
    virtual bool NeedRebuildRenderGraph()
    {
        return false;
    }
    virtual void OnImGuiUpdate()
    {

    }
   
};
} // namespace Aether