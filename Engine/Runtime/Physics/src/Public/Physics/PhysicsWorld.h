#pragma once

#include "Physics/Shape.h"
#include <memory>

namespace Aether::Physics
{

class PhysicsWorld final
{
public:
    struct Impl;

    PhysicsWorld(const PhysicsWorld&) = delete;
    PhysicsWorld& operator=(const PhysicsWorld&) = delete;
    PhysicsWorld(PhysicsWorld&&) noexcept;
    PhysicsWorld& operator=(PhysicsWorld&&) noexcept;
    ~PhysicsWorld();

    Result<BodyId> CreateBody(const BodyDesc& desc);
    Result<void> DestroyBody(BodyId body);
    [[nodiscard]] bool IsBodyValid(BodyId body) const;

    Result<WorldTransform> GetTransform(BodyId body) const;
    Result<void> SetTransform(BodyId body, const WorldTransform& transform, bool activate = true);
    Result<Vec3f> GetLinearVelocity(BodyId body) const;
    Result<void> SetLinearVelocity(BodyId body, const Vec3f& velocity);
    Result<void> AddForce(BodyId body, const Vec3f& force);
    Result<void> AddImpulse(BodyId body, const Vec3f& impulse);
    Result<void> MoveKinematic(BodyId body, const WorldTransform& target, float deltaTime);
    Result<UserData> GetUserData(BodyId body) const;

    Result<void> Step(float deltaTime, std::uint32_t collisionSteps = 1);
    void OptimizeBroadPhase();

    Result<std::optional<RayCastHit>> CastRayClosest(const RayCast& cast,
                                                     const QueryFilter& filter = {}) const;
    Result<std::optional<ShapeCastHit>> CastShapeClosest(const ShapeCast& cast,
                                                         const ShapeCastOptions& options = {}) const;
    Result<bool> CastShapeAny(const ShapeCast& cast,
                              const ShapeCastOptions& options = {}) const;
    Result<std::vector<OverlapHit>> OverlapShape(const ShapeOverlap& overlap,
                                                 const QueryFilter& filter = {}) const;

private:
    std::unique_ptr<Impl> m_Impl;
    explicit PhysicsWorld(std::unique_ptr<Impl> impl);
    friend class PhysicsEngine;
};

} // namespace Aether::Physics
