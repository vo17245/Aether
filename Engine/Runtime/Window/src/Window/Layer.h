#pragma once
#include "Window/Event.h"
#include <Render/Render.h>
#include <Render/Threads/RenderCommand.h>
#include <ImGui/Core/imgui.h>
#include <memory>
#include <functional>
#include <stdexcept>
#include <utility>
#include <vector>
namespace Aether {
class Window;

// Owns resource commands between main-thread extraction and atomic envelope
// acceptance.  Notifications must own their state and must not borrow a Layer.
class RenderCommandExtraction
{
public:
    using Notification = std::function<void()>;

    RenderCommandExtraction() = default;
    ~RenderCommandExtraction()
    {
        try { Cancel(); } catch (...) {}
    }
    RenderCommandExtraction(RenderCommandExtraction&&) noexcept = default;
    RenderCommandExtraction& operator=(RenderCommandExtraction&&) noexcept = default;
    RenderCommandExtraction(const RenderCommandExtraction&) = delete;
    RenderCommandExtraction& operator=(const RenderCommandExtraction&) = delete;

    void Add(std::unique_ptr<Render::IRenderCommand> command,
             Notification accepted = {}, Notification cancelled = {})
    {
        if (!command)
            throw std::invalid_argument("RenderCommandExtraction requires an owned command");
        m_Commands.push_back(std::move(command));
        m_Accepted.push_back(std::move(accepted));
        m_Cancelled.push_back(std::move(cancelled));
    }

    bool Empty() const noexcept { return m_Commands.empty(); }
    std::vector<std::unique_ptr<Render::IRenderCommand>> TakeCommands()
    {
        return std::move(m_Commands);
    }
    void Accept()
    {
        if (m_Resolved) return;
        m_Resolved = true;
        for (auto& notify : m_Accepted) if (notify) notify();
        m_Cancelled.clear();
    }
    void Cancel()
    {
        if (m_Resolved) return;
        m_Resolved = true;
        for (auto& notify : m_Cancelled) if (notify) notify();
        m_Accepted.clear();
        m_Commands.clear();
    }

private:
    std::vector<std::unique_ptr<Render::IRenderCommand>> m_Commands;
    std::vector<Notification> m_Accepted;
    std::vector<Notification> m_Cancelled;
    bool m_Resolved = false;
};

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
    virtual void ExtractRenderCommands(RenderCommandExtraction& commands)
    {
        (void)commands;
    }
    // Features contain render-owner state and outlive the frontend Layer when
    // retained by an accepted frame or an old RenderGraph.
    virtual void CollectRenderFeatures(std::vector<std::shared_ptr<Render::RenderFeature>>& features)
    {
        (void)features;
    }

};
} // namespace Aether
