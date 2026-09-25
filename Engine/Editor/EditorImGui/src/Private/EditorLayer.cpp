#include <EditorImGui/EditorLayer.h>

namespace Aether::EditorImGui
{
void EditorLayer::OnAttach(Window* window)
{
    m_Window = window;
    if (m_Callbacks.onAttach) m_Callbacks.onAttach(window);
}

void EditorLayer::OnDetach()
{
    if (m_Callbacks.onDetach) m_Callbacks.onDetach();
    m_Window = nullptr;
}

void EditorLayer::OnEvent(Event& event)
{
    m_Host.OnEvent(event);
    if (m_Callbacks.onEvent) m_Callbacks.onEvent(event);
}

void EditorLayer::OnUpdate(float seconds)
{
    m_Host.Tick(seconds);
    if (m_Callbacks.onUpdate) m_Callbacks.onUpdate(seconds);
}

void EditorLayer::OnFrameBegin()
{
    if (m_Callbacks.onFrameBegin) m_Callbacks.onFrameBegin();
}

void EditorLayer::OnBuildRenderGraph(RenderGraph::RenderGraph& graph)
{
    m_Host.OnBuildRenderGraph(graph);
    if (m_Callbacks.onBuildRenderGraph) m_Callbacks.onBuildRenderGraph(graph);
}

bool EditorLayer::NeedRebuildRenderGraph()
{
    return m_Host.NeedRebuildRenderGraph() ||
        (m_Callbacks.needRebuildRenderGraph && m_Callbacks.needRebuildRenderGraph());
}

void EditorLayer::OnImGuiUpdate()
{
    m_Systems.Draw(m_Host.GetWorld(), m_Intents);
}

void EditorLayer::OnUpload(PendingUploadList& uploads)
{
    m_Host.OnUpload(uploads);
    if (m_Callbacks.onUpload) m_Callbacks.onUpload(uploads);
}

void EditorLayer::ExtractRenderData(Render::RenderFeatureFrame& frame)
{
    m_Host.ExtractRenderData(frame);
    if (m_Callbacks.extractRenderData) m_Callbacks.extractRenderData(frame);
}

void EditorLayer::ExtractRenderCommands(RenderCommandExtraction& commands)
{
    if (m_Callbacks.extractRenderCommands) m_Callbacks.extractRenderCommands(commands);
}

void EditorLayer::CollectRenderFeatures(std::vector<std::shared_ptr<Render::RenderFeature>>& features)
{
    if (m_Callbacks.collectRenderFeatures) m_Callbacks.collectRenderFeatures(features);
}
} // namespace Aether::EditorImGui
