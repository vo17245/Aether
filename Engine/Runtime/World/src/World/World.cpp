#include "World.h"
#include "System.h"
namespace Aether
{

void World::OnUpdate(float deltaTime)
{
    for (auto& system : m_Systems)
    {
        system->OnUpdate(deltaTime);
    }
}
bool World::NeedRebuildRenderGraph()
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
void World::OnUpload(PendingUploadList& uploadList)
{
    for (auto& system : m_Systems)
    {
        system->OnUpload(uploadList);
    }
}
void World::OnEvent(Event& event)
{
    for (auto& system : m_Systems)
    {
        system->OnEvent(event);
    }
}
void World::PushSystem(Scope<System>&& system)
{
    system->OnAttach(this);
    m_Systems.push_back(std::move(system));
}
void World::EraseSystem(System* system)
{
    auto iter = std::find_if(m_Systems.begin(), m_Systems.end(), [&](const Scope<System>& ptr) { return ptr.get() == system; });
    if (iter != m_Systems.end())
    {
        (*iter)->OnDetach();
        m_Systems.erase(iter);
    }
}
void World::OnBuildRenderGraph(RenderGraph::RenderGraph& renderGraph)
{
    for (auto& system : m_Systems)
    {
        system->OnBuildRenderGraph(renderGraph);
    }
}
bool BuildExecutionOrder()
{
    return false;
}
} // namespace Aether