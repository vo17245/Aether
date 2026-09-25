#pragma once

#include <EditorFramework/EditorHost.h>
#include <EditorImGui/UiSystemRegistry.h>
#include <Window/Layer.h>

#include <functional>

namespace Aether::EditorImGui
{
struct EditorLayerCallbacks
{
    std::function<void(Window*)> onAttach;
    std::function<void()> onDetach;
    std::function<void(Event&)> onEvent;
    std::function<void(float)> onUpdate;
    std::function<void()> onFrameBegin;
    std::function<void(RenderGraph::RenderGraph&)> onBuildRenderGraph;
    std::function<bool()> needRebuildRenderGraph;
    std::function<void(PendingUploadList&)> onUpload;
    std::function<void(Render::RenderFeatureFrame&)> extractRenderData;
    std::function<void(RenderCommandExtraction&)> extractRenderCommands;
    std::function<void(std::vector<std::shared_ptr<Render::RenderFeature>>&)> collectRenderFeatures;
};

class EditorLayer final : public Layer
{
public:
    EditorLayer(EditorFramework::EditorHost& host, UiSystemRegistry& systems, UiIntentSink& intents,
                EditorLayerCallbacks callbacks = {})
        : m_Host(host), m_Systems(systems), m_Intents(intents), m_Callbacks(std::move(callbacks)) {}
    void OnAttach(Window* window) override;
    void OnDetach() override;
    void OnEvent(Event& event) override;
    void OnUpdate(float sec) override;
    void OnFrameBegin() override;
    void OnBuildRenderGraph(RenderGraph::RenderGraph& graph) override;
    bool NeedRebuildRenderGraph() override;
    void OnImGuiUpdate() override;
    void OnUpload(PendingUploadList& uploads) override;
    void ExtractRenderData(Render::RenderFeatureFrame& frame) override;
    void ExtractRenderCommands(RenderCommandExtraction& commands) override;
    void CollectRenderFeatures(std::vector<std::shared_ptr<Render::RenderFeature>>& features) override;
private:
    EditorFramework::EditorHost& m_Host;
    UiSystemRegistry& m_Systems;
    UiIntentSink& m_Intents;
    EditorLayerCallbacks m_Callbacks;
    Window* m_Window = nullptr;
};
}
