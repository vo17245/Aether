#include <EditorFramework/EditorHost.h>
#include <World/System.h>
#include <EditorFramework/Commands.h>
#include <EditorFramework/AssetImport.h>
#include <EditorFramework/PlayLifecycle.h>

namespace Aether::EditorFramework
{
EditorHost::EditorHost() : m_World(std::make_unique<World>())
{
    m_Session = m_World->CreateEntity();
    m_World->AddComponent<EditorSessionComponent>(m_Session);
    m_World->AddComponent<ProjectStateComponent>(m_Session);
}

EditorHost::~EditorHost()
{
    Shutdown();
}

void EditorHost::Tick(float deltaTime)
{
    if (!m_World || m_Session == entt::null || !m_World->IsValid(m_Session)) return;
    if (m_World->GetComponent<EditorSessionComponent>(m_Session).closing) return;
    CommitReadyImports(*m_World);
    ApplyEditCommands(*m_World);
    m_World->OnUpdatePhase(SystemUpdatePhase::EditorInput, deltaTime);
    m_World->OnUpdatePhase(SystemUpdatePhase::EditorTools, deltaTime);
    for (const auto entity : m_World->Select<WorldDocumentComponent>())
    {
        auto& document = m_World->GetComponent<WorldDocumentComponent>(entity);
        if (document.authoringWorld)
            document.authoringWorld->OnUpdatePhase(SystemUpdatePhase::Presentation, deltaTime);
    }
    for (const auto entity : m_World->Select<PlaySessionComponent>())
    {
        auto& session = m_World->GetComponent<PlaySessionComponent>(entity);
        if (session.state == PlayState::Playing)
            (void)AdvancePlay(session, deltaTime);
        if ((session.state == PlayState::Playing || session.state == PlayState::Paused) && session.playWorld)
            session.playWorld->OnUpdatePhase(SystemUpdatePhase::Presentation, deltaTime);
    }
    m_World->OnUpdatePhase(SystemUpdatePhase::Presentation, deltaTime);
    m_World->OnUpdatePhase(SystemUpdatePhase::EditorModel, deltaTime);
}

void EditorHost::OnEvent(Event& event)
{
    if (!m_World) return;
    m_World->OnEvent(event);
}

void EditorHost::OnBuildRenderGraph(RenderGraph::RenderGraph& graph)
{
    if (!m_World) return;
    m_World->OnBuildRenderGraph(graph);
    for (const auto entity : m_World->Select<WorldDocumentComponent>())
    {
        auto& document = m_World->GetComponent<WorldDocumentComponent>(entity);
        if (document.authoringWorld) document.authoringWorld->OnBuildRenderGraph(graph);
    }
    for (const auto entity : m_World->Select<PlaySessionComponent>())
    {
        auto& session = m_World->GetComponent<PlaySessionComponent>(entity);
        if (session.playWorld && session.state != PlayState::Stopped && session.state != PlayState::Failed)
            session.playWorld->OnBuildRenderGraph(graph);
    }
}

bool EditorHost::NeedRebuildRenderGraph()
{
    if (!m_World) return false;
    if (m_World->NeedRebuildRenderGraph()) return true;
    for (const auto entity : m_World->Select<WorldDocumentComponent>())
    {
        const auto& document = m_World->GetComponent<WorldDocumentComponent>(entity);
        if (document.authoringWorld && document.authoringWorld->NeedRebuildRenderGraph()) return true;
    }
    for (const auto entity : m_World->Select<PlaySessionComponent>())
    {
        const auto& session = m_World->GetComponent<PlaySessionComponent>(entity);
        if (session.playWorld && session.state != PlayState::Stopped && session.state != PlayState::Failed &&
            session.playWorld->NeedRebuildRenderGraph()) return true;
    }
    return false;
}

void EditorHost::OnUpload(PendingUploadList& uploads)
{
    if (!m_World) return;
    m_World->OnUpload(uploads);
    for (const auto entity : m_World->Select<WorldDocumentComponent>())
    {
        auto& document = m_World->GetComponent<WorldDocumentComponent>(entity);
        if (document.authoringWorld) document.authoringWorld->OnUpload(uploads);
    }
    for (const auto entity : m_World->Select<PlaySessionComponent>())
    {
        auto& session = m_World->GetComponent<PlaySessionComponent>(entity);
        if (session.playWorld && session.state != PlayState::Stopped && session.state != PlayState::Failed)
            session.playWorld->OnUpload(uploads);
    }
}

void EditorHost::ExtractRenderData(Render::RenderFeatureFrame& frame)
{
    if (!m_World) return;
    m_World->ExtractRenderData(frame);
    for (const auto entity : m_World->Select<WorldDocumentComponent>())
    {
        auto& document = m_World->GetComponent<WorldDocumentComponent>(entity);
        if (document.authoringWorld) document.authoringWorld->ExtractRenderData(frame);
    }
    for (const auto entity : m_World->Select<PlaySessionComponent>())
    {
        auto& session = m_World->GetComponent<PlaySessionComponent>(entity);
        if (session.playWorld && session.state != PlayState::Stopped && session.state != PlayState::Failed)
            session.playWorld->ExtractRenderData(frame);
    }
}

void EditorHost::Shutdown()
{
    if (!m_World) return;
    if (m_Session != entt::null && m_World->IsValid(m_Session))
        m_World->GetComponent<EditorSessionComponent>(m_Session).closing = true;
    m_World.reset();
    m_Session = entt::null;
}
}
