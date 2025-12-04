#include "Scene.h"
#include "System.h"
namespace Aether
{

void Scene::OnUpdate(float deltaTime)
{
    for (auto& system : m_Systems)
    {
        system->OnUpdate(deltaTime);
    }
}
bool Scene::NeedRebuildRenderGraph()
{
    for (auto& system : m_Systems)
    {
        if (system->NeedRebuildRenderGraph())
        {
            return true;
        }
    }
    return false;
}
void Scene::OnUpload(PendingUploadList& uploadList)
{
    for (auto& system : m_Systems)
    {
        system->OnUpload(uploadList);
    }
}
void Scene::OnEvent(Event& event)
{
    for (auto& system : m_Systems)
    {
        system->OnEvent(event);
    }
}
void Scene::PushSystem(Scope<System>&& system)
{
    system->OnAttach(this);
    m_Systems.push_back(std::move(system));
}
void Scene::EraseSystem(System* system)
{
    auto iter = std::find_if(m_Systems.begin(), m_Systems.end(), [&](const Scope<System>& ptr) { return ptr.get() == system; });
    if (iter != m_Systems.end())
    {
        (*iter)->OnDetach();
        m_Systems.erase(iter);
    }
}
void Scene::OnBuildRenderGraph(RenderGraph::RenderGraph& renderGraph)
{
    for (auto& system : m_Systems)
    {
        system->OnBuildRenderGraph(renderGraph);
    }
}
} // namespace Aether