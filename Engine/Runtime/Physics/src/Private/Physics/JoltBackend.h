#pragma once

#include <Jolt/Jolt.h>
#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Physics/Body/BodyFilter.h>
#include <Jolt/Physics/Body/BodyInterface.h>
#include <Jolt/Physics/Collision/BroadPhase/BroadPhaseLayer.h>
#include <Jolt/Physics/Collision/ObjectLayer.h>
#include <Jolt/Physics/Collision/Shape/Shape.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/Physics/Collision/Shape/SubShapeID.h>

#include "Physics/PhysicsEngine.h"
#include <mutex>
#include <thread>
#include <unordered_map>

namespace Aether::Physics
{

struct RuntimeState
{
    explicit RuntimeState(TraceCallback callback) : trace(std::move(callback)) {}
    ~RuntimeState();

    TraceCallback trace;
};

Result<std::shared_ptr<RuntimeState>> AcquireRuntime(const EngineDesc& desc);

struct CompoundMeta
{
    std::uint32_t userData = 0;
};

struct Shape::Impl
{
    std::shared_ptr<RuntimeState> runtime;
    JPH::RefConst<JPH::Shape> shape;
    ShapeKind kind = ShapeKind::Box;
    bool mustBeStatic = false;
    std::vector<CompoundMeta> directChildren;
};

struct LayerTable
{
    std::array<CollisionMask, MAX_COLLISION_LAYERS> collidesWith{};
    std::array<bool, MAX_COLLISION_LAYERS> declared{};

    [[nodiscard]] bool IsDeclared(CollisionLayer layer) const
    {
        return layer < MAX_COLLISION_LAYERS && declared[layer];
    }
};

Result<LayerTable> BuildLayerTable(const WorldDesc& desc);
[[nodiscard]] JPH::ObjectLayer EncodeObjectLayer(CollisionLayer layer, MotionType motion);
[[nodiscard]] CollisionLayer DecodeLogicalLayer(JPH::ObjectLayer layer);
[[nodiscard]] bool IsMovingObjectLayer(JPH::ObjectLayer layer);

class BroadPhaseLayerInterface final : public JPH::BroadPhaseLayerInterface
{
public:
    uint GetNumBroadPhaseLayers() const override { return 2; }
    JPH::BroadPhaseLayer GetBroadPhaseLayer(JPH::ObjectLayer layer) const override
    {
        return JPH::BroadPhaseLayer(IsMovingObjectLayer(layer) ? 1 : 0);
    }

#if defined(JPH_EXTERNAL_PROFILE) || defined(JPH_PROFILE_ENABLED)
    const char* GetBroadPhaseLayerName(JPH::BroadPhaseLayer layer) const override
    {
        return layer == JPH::BroadPhaseLayer(0) ? "NON_MOVING" : "MOVING";
    }
#endif
};

class ObjectVsBroadPhaseLayerFilter final : public JPH::ObjectVsBroadPhaseLayerFilter
{
public:
    bool ShouldCollide(JPH::ObjectLayer objectLayer, JPH::BroadPhaseLayer broadLayer) const override
    {
        return IsMovingObjectLayer(objectLayer) || broadLayer == JPH::BroadPhaseLayer(1);
    }
};

class ObjectLayerPairFilter final : public JPH::ObjectLayerPairFilter
{
public:
    explicit ObjectLayerPairFilter(const LayerTable& table) : m_Table(table) {}

    bool ShouldCollide(JPH::ObjectLayer first, JPH::ObjectLayer second) const override
    {
        const auto firstLayer = DecodeLogicalLayer(first);
        const auto secondLayer = DecodeLogicalLayer(second);
        return m_Table.IsDeclared(firstLayer) && m_Table.IsDeclared(secondLayer)
            && (m_Table.collidesWith[firstLayer] & LayerBit(secondLayer)) != 0
            && (m_Table.collidesWith[secondLayer] & LayerBit(firstLayer)) != 0;
    }

private:
    const LayerTable& m_Table;
};

class AllBroadPhaseLayerFilter final : public JPH::BroadPhaseLayerFilter
{
public:
    bool ShouldCollide(JPH::BroadPhaseLayer) const override { return true; }
};

class QueryObjectLayerFilter final : public JPH::ObjectLayerFilter
{
public:
    explicit QueryObjectLayerFilter(CollisionMask mask) : m_Mask(mask) {}

    bool ShouldCollide(JPH::ObjectLayer layer) const override
    {
        return (m_Mask & LayerBit(DecodeLogicalLayer(layer))) != 0;
    }

private:
    CollisionMask m_Mask;
};

class QueryBodyFilter final : public JPH::BodyFilter
{
public:
    QueryBodyFilter(const QueryFilter& filter, const JPH::BodyID& ignored) :
        m_Filter(filter), m_Ignored(ignored)
    {
    }

    bool ShouldCollide(const JPH::BodyID& id) const override
    {
        return !m_Filter.ignoredBody || id != m_Ignored;
    }

    bool ShouldCollideLocked(const JPH::Body& body) const override;

private:
    const QueryFilter& m_Filter;
    JPH::BodyID m_Ignored;
};

struct PhysicsWorld::Impl
{
    std::shared_ptr<RuntimeState> runtime;
    WorldDesc desc;
    LayerTable layers;
    BroadPhaseLayerInterface broadPhaseLayers;
    ObjectVsBroadPhaseLayerFilter objectVsBroadPhaseFilter;
    ObjectLayerPairFilter objectLayerPairFilter;
    JPH::PhysicsSystem system;
    std::unique_ptr<JPH::TempAllocatorImpl> tempAllocator;
    std::unique_ptr<JPH::JobSystemThreadPool> jobSystem;
    std::unordered_map<std::uint32_t, std::shared_ptr<const Shape::Impl>> bodyShapes;
    std::thread::id ownerThread;

    Impl(std::shared_ptr<RuntimeState> state, WorldDesc worldDesc, LayerTable table) :
        runtime(std::move(state)), desc(std::move(worldDesc)), layers(table), objectLayerPairFilter(layers),
        ownerThread(std::this_thread::get_id())
    {
    }

    ~Impl();

    [[nodiscard]] bool OnOwnerThread() const noexcept
    {
        return ownerThread == std::this_thread::get_id();
    }

    [[nodiscard]] Result<void> ValidateOwner() const;
    [[nodiscard]] Result<JPH::BodyID> ToJoltBody(BodyId body) const;
};

struct PhysicsEngine::Impl
{
    std::shared_ptr<RuntimeState> runtime;
};

} // namespace Aether::Physics
