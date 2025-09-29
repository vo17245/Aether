#pragma once
#include <entt/entt.hpp>
#include <Render/Render.h>
#include <Window/Event.h>
namespace Aether
{
class System;
using EntityId = entt::entity;
class Scene
{
public:
    EntityId CreateEntity()
    {
        auto entity = m_Registry.create();
        return entity;
    }
    void DestroyEntity(EntityId entity)
    {
        m_Registry.destroy(entity);
    }
    template <typename T, typename... Args>
    T& AddComponent(EntityId entity, Args&&... args)
    {
        return m_Registry.emplace<T>(entity, std::forward<Args>(args)...);
    }
    template <typename T>
    void RemoveComponent(EntityId entity)
    {
        m_Registry.remove<T>(entity);
    }
    template <typename T>
    T& GetComponent(EntityId entity)
    {
        return m_Registry.get<T>(entity);
    }
    template <typename T>
    bool HasComponent(EntityId entity)
    {
        return m_Registry.any_of<T>(entity);
    }
    template <typename... Ts>
    auto Select()
    {
        return m_Registry.view<Ts...>();
    }
public:
    void PushSystem(Scope<System>&& system);
    void EraseSystem(System* system);
    void OnUpdate(float deltaTime);
    bool NeedRebuildRenderGraph();
    void OnUpload(PendingUploadList& uploadList);
    void OnEvent(Event& event);
    void OnBuildRenderGraph(RenderGraph::RenderGraph& renderGraph);
private:
    entt::registry m_Registry;
    std::vector<Scope<System>> m_Systems;
};
} // namespace Aether