#pragma once

#include "Core/Math.h"
#include "Physics/Error.h"
#include <cstdint>
#include <functional>
#include <limits>
#include <memory>
#include <optional>
#include <span>
#include <vector>

namespace Aether::Physics
{

using Aether::Quatf;
using Aether::Vec3d;
using Aether::Vec3f;

struct WorldTransform
{
    Vec3d position = Vec3d::Zero();
    Quatf rotation = Quatf::Identity();
};

struct LocalTransform
{
    Vec3f position = Vec3f::Zero();
    Quatf rotation = Quatf::Identity();
};

struct BodyId
{
    std::uint32_t value = std::numeric_limits<std::uint32_t>::max();

    [[nodiscard]] bool IsValid() const noexcept
    {
        return value != std::numeric_limits<std::uint32_t>::max();
    }

    auto operator<=>(const BodyId&) const = default;
};

struct BodyIdHash
{
    std::size_t operator()(BodyId id) const noexcept
    {
        return std::hash<std::uint32_t>{}(id.value);
    }
};

using UserData = std::uint64_t;
using CollisionLayer = std::uint8_t;
using CollisionMask = std::uint32_t;
inline constexpr CollisionLayer MAX_COLLISION_LAYERS = 32;

[[nodiscard]] constexpr CollisionMask LayerBit(CollisionLayer layer) noexcept
{
    return layer < MAX_COLLISION_LAYERS ? CollisionMask{1u} << layer : CollisionMask{0};
}

struct CollisionLayerDesc
{
    CollisionLayer layer = 0;
    CollisionMask collidesWith = 0;
};

struct BoxGeometry
{
    Vec3f halfExtent = Vec3f::Constant(0.5f);
    float convexRadius = 0.0f;
};

struct SphereGeometry
{
    float radius = 0.5f;
};

struct CapsuleGeometry
{
    float radius = 0.5f;
    float height = 2.0f;
};

struct TriangleMeshDesc
{
    // Triangles are single-sided: indices must be counter-clockwise when viewed
    // from the front face. Querying a back face is ignored by the backend.
    std::span<const Vec3f> vertices;
    std::span<const std::uint32_t> indices;
};

enum class ShapeKind
{
    Box,
    Sphere,
    Capsule,
    StaticCompound,
    TriangleMesh
};

enum class MotionType
{
    Static,
    Kinematic,
    Dynamic
};

enum class MotionQuality
{
    Discrete,
    LinearCast
};

struct QueryFilter
{
    CollisionMask layers = std::numeric_limits<CollisionMask>::max();
    std::optional<BodyId> ignoredBody;
    bool includeSensors = false;
};

struct RayCast
{
    Vec3d origin = Vec3d::Zero();
    Vec3f displacement = Vec3f::Zero();
};

class Shape final
{
public:
    struct Impl;

    Shape() = default;
    Shape(const Shape&) noexcept = default;
    Shape& operator=(const Shape&) noexcept = default;
    Shape(Shape&&) noexcept = default;
    Shape& operator=(Shape&&) noexcept = default;
    ~Shape();

    [[nodiscard]] bool IsValid() const noexcept;
    [[nodiscard]] ShapeKind Kind() const;

private:
    std::shared_ptr<const Impl> m_Impl;
    explicit Shape(std::shared_ptr<const Impl> impl) : m_Impl(std::move(impl)) {}
    friend class PhysicsEngine;
    friend class PhysicsWorld;
};

struct ShapeCast
{
    Shape shape;
    WorldTransform start;
    Vec3f displacement = Vec3f::Zero();
};

struct ShapeCastOptions
{
    QueryFilter filter;
    bool useShrunkenShapeAndConvexRadius = true;
    bool returnDeepestPointForInitialOverlap = true;
};

struct ShapeOverlap
{
    Shape shape;
    WorldTransform transform;
};

struct RayCastHit
{
    BodyId body;
    UserData bodyUserData = 0;
    std::uint32_t subShapeUserData = 0;
    Vec3d point = Vec3d::Zero();
    Vec3f normal = Vec3f::Zero();
    float fraction = 0.0f;
    float distance = 0.0f;
};

struct ShapeCastHit
{
    BodyId body;
    UserData bodyUserData = 0;
    std::uint32_t subShapeUserData = 0;
    Vec3d pointOnTarget = Vec3d::Zero();
    Vec3d castOriginAtHit = Vec3d::Zero();
    Vec3f normal = Vec3f::Zero();
    float fraction = 0.0f;
    float distance = 0.0f;
    float penetrationDepth = 0.0f;
    bool startedOverlapping = false;
};

struct OverlapHit
{
    BodyId body;
    UserData bodyUserData = 0;
    std::uint32_t subShapeUserData = 0;
    Vec3f separationDirection = Vec3f::Zero();
    float penetrationDepth = 0.0f;
};

struct WorldDesc
{
    Vec3f gravity{0.0f, -9.81f, 0.0f};
    std::uint32_t maxBodies = 65536;
    std::uint32_t numBodyMutexes = 0;
    std::uint32_t maxBodyPairs = 65536;
    std::uint32_t maxContactConstraints = 10240;
    std::size_t tempAllocatorBytes = 16 * 1024 * 1024;
    std::uint32_t workerThreads = 0;
    std::vector<CollisionLayerDesc> layers = {
        {0, LayerBit(0) | LayerBit(1)},
        {1, LayerBit(0) | LayerBit(1)}
    };
};

struct BodyDesc
{
    Shape shape;
    WorldTransform transform;
    MotionType motionType = MotionType::Static;
    MotionQuality motionQuality = MotionQuality::Discrete;
    CollisionLayer layer = 0;
    UserData userData = 0;
    Vec3f linearVelocity = Vec3f::Zero();
    Vec3f angularVelocity = Vec3f::Zero();
    float mass = 1.0f;
    float friction = 0.2f;
    float restitution = 0.0f;
    float linearDamping = 0.05f;
    float angularDamping = 0.05f;
    float gravityFactor = 1.0f;
    bool allowSleeping = true;
    bool startActive = true;
    bool isSensor = false;
};

} // namespace Aether::Physics
