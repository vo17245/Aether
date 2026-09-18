#pragma once

#include "Physics/PhysicsWorld.h"
#include <functional>
#include <memory>
#include <string_view>

namespace Aether::Physics
{

using TraceCallback = std::function<void(std::string_view)>;

struct EngineDesc
{
    TraceCallback trace;
};

class PhysicsEngine final
{
public:
    static Result<std::unique_ptr<PhysicsEngine>> Create(const EngineDesc& desc = {});

    PhysicsEngine(const PhysicsEngine&) = delete;
    PhysicsEngine& operator=(const PhysicsEngine&) = delete;
    PhysicsEngine(PhysicsEngine&&) noexcept;
    PhysicsEngine& operator=(PhysicsEngine&&) noexcept;
    ~PhysicsEngine();

    Result<Shape> CreateBox(const BoxGeometry& geometry) const;
    Result<Shape> CreateSphere(const SphereGeometry& geometry) const;
    Result<Shape> CreateCapsule(const CapsuleGeometry& geometry) const;
    Result<Shape> CreateStaticCompound(std::span<const CompoundChild> children) const;
    Result<Shape> CreateTriangleMesh(const TriangleMeshDesc& desc) const;
    Result<std::unique_ptr<PhysicsWorld>> CreateWorld(const WorldDesc& desc = {}) const;

private:
    struct Impl;
    std::unique_ptr<Impl> m_Impl;
    explicit PhysicsEngine(std::unique_ptr<Impl> impl);
};

} // namespace Aether::Physics
