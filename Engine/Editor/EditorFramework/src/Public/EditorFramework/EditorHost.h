#pragma once

#include <EditorFramework/EditorComponents.h>

#include <memory>

namespace Aether::EditorFramework
{
class EditorHost
{
public:
    EditorHost();
    EditorHost(const EditorHost&) = delete;
    EditorHost& operator=(const EditorHost&) = delete;
    ~EditorHost();

    void Tick(float deltaTime);
    void OnEvent(Event& event);
    void OnBuildRenderGraph(RenderGraph::RenderGraph& graph);
    bool NeedRebuildRenderGraph();
    void OnUpload(PendingUploadList& uploads);
    void ExtractRenderData(Render::RenderFeatureFrame& frame);
    void Shutdown();
    World& GetWorld() noexcept { return *m_World; }
    const World& GetWorld() const noexcept { return *m_World; }
    EntityId SessionEntity() const noexcept { return m_Session; }
private:
    std::unique_ptr<World> m_World;
    EntityId m_Session = entt::null;
};
}
