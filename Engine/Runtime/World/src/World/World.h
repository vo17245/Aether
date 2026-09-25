#pragma once
#include <entt/entt.hpp>
#include <World/Serialization/ArchiveTypes.h>
#include <Render/Render.h>
#include <Window/Event.h>
#include <string>
#include <string_view>
#include <vector>
#include <memory>
#include <cstdint>
namespace Aether
{
class System;
enum class SystemUpdatePhase : std::uint8_t;
namespace Serialization { class ComponentCodecRegistry; class SaveContext; class LoadContext; }
using EntityId = entt::entity;
class World
{
public:
    World();
    ~World();
    World(const World&) = delete;
    World& operator=(const World&) = delete;
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
    const T& GetComponent(EntityId entity) const
    {
        return m_Registry.get<T>(entity);
    }
    template <typename T>
    bool HasComponent(EntityId entity)
    {
        return m_Registry.any_of<T>(entity);
    }
    template <typename T>
    bool HasComponent(EntityId entity) const
    {
        return m_Registry.any_of<T>(entity);
    }
    template <typename... Ts>
    auto Select()
    {
        return m_Registry.view<Ts...>();
    }
    template <typename... Ts>
    auto Select() const
    {
        return m_Registry.view<Ts...>();
    }
    bool IsValid(EntityId entity) const { return m_Registry.valid(entity); }
    bool IsDispatching() const noexcept { return m_DispatchDepth != 0; }
    std::vector<EntityId> Entities() const
    {
        std::vector<EntityId> entities;
        const auto* pool = m_Registry.storage<EntityId>();
        entities.reserve(pool ? pool->size() : 0);
        if (pool)
            for (const auto& item : pool->each()) entities.push_back(std::get<0>(item));
        return entities;
    }
    // Serialization is synchronous. Callers must provide exclusive, single-threaded
    // access and must not call it from a System callback.
    Serialization::Result<Json> Serialize(const Serialization::ComponentCodecRegistry& codecs,
                                          Serialization::SaveContext& context) const;
    static Serialization::Result<std::unique_ptr<World>> Deserialize(
        const Json& document, const Serialization::ComponentCodecRegistry& codecs,
        Serialization::LoadContext& context);
public:
    void PushSystem(Scope<System>&& system);
    void EraseSystem(System* system);
	void OnUpdate(float deltaTime);
	void OnUpdatePhase(SystemUpdatePhase phase, float deltaTime);
    bool NeedRebuildRenderGraph();
    void OnUpload(PendingUploadList& uploadList);
    void OnEvent(Event& event);
    void OnBuildRenderGraph(RenderGraph::RenderGraph& renderGraph);
    void OnFrameBegin(std::uint32_t frameSlot);
    void ExtractRenderData(Render::RenderFeatureFrame& frame);
	void BuildExecutionOrder();
	void BuildPhaseExecutionOrder();
	void ReplaceDataFrom(World& detached);
    std::vector<std::string_view> ExecutionOrderSignatures();
private:
    friend class WorldArchiveAccess;
    void EnsureExecutionOrder();
    template <typename Callback>
    void Dispatch(Callback&& callback);

    entt::registry m_Registry;
    std::vector<Scope<System>> m_Systems;
    std::vector<System*> m_ExecutionOrder;
	bool m_OrderDirty = true;
	bool m_PhaseOrderValid = false;
    std::uint32_t m_DispatchDepth = 0;
};
} // namespace Aether
