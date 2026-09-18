#include "JoltBackend.h"
#include "JoltConversion.h"

#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/CapsuleShape.h>
#include <Jolt/Physics/Collision/Shape/MeshShape.h>
#include <Jolt/Physics/Collision/Shape/RotatedTranslatedShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/Collision/Shape/StaticCompoundShape.h>
#include <cmath>

namespace Aether::Physics
{
namespace
{
Error Invalid(std::string message)
{
    return {ErrorCode::InvalidArgument, std::move(message)};
}

bool Finite(const Vec3f& value)
{
    return value.allFinite();
}

bool ValidRotation(const Quatf& rotation)
{
    return rotation.coeffs().allFinite() && std::abs(rotation.norm() - 1.0f) <= 1e-4f;
}

bool ValidLocalTransform(const LocalTransform& transform)
{
    return Finite(transform.position) && ValidRotation(transform.rotation);
}

Result<std::shared_ptr<Shape::Impl>> ShapeResultToImpl(const std::shared_ptr<RuntimeState>& runtime,
                                                        JPH::ShapeSettings::ShapeResult result,
                                                        ShapeKind kind,
                                                        bool mustBeStatic,
                                                        std::vector<CompoundMeta> children = {})
{
    if (!result.IsValid())
    {
        const auto message = result.HasError() ? std::string(result.GetError().c_str()) : "Jolt returned an invalid shape";
        return std::unexpected(Error{ErrorCode::ShapeCreationFailed, message});
    }
    auto impl = std::make_shared<Shape::Impl>();
    impl->runtime = runtime;
    impl->shape = result.Get();
    impl->kind = kind;
    impl->mustBeStatic = mustBeStatic;
    impl->directChildren = std::move(children);
    return impl;
}
} // namespace

Shape::~Shape() = default;

bool Shape::IsValid() const noexcept
{
    return m_Impl != nullptr && m_Impl->shape != nullptr;
}

ShapeKind Shape::Kind() const
{
    return m_Impl != nullptr ? m_Impl->kind : ShapeKind::Box;
}

Result<Shape> PhysicsEngine::CreateBox(const BoxGeometry& geometry) const
{
    if (!m_Impl || !m_Impl->runtime || !Finite(geometry.halfExtent) || !std::isfinite(geometry.convexRadius)
        || geometry.halfExtent.minCoeff() <= 0.0f || geometry.convexRadius < 0.0f
        || geometry.convexRadius > geometry.halfExtent.minCoeff())
        return std::unexpected(Invalid("invalid box geometry"));
    try
    {
        JPH::BoxShapeSettings settings(ToJolt(geometry.halfExtent), geometry.convexRadius);
        auto impl = ShapeResultToImpl(m_Impl->runtime, settings.Create(), ShapeKind::Box, false);
        if (!impl)
            return std::unexpected(impl.error());
        return Shape{std::move(*impl)};
    }
    catch (const std::bad_alloc&)
    {
        return std::unexpected(Error{ErrorCode::OutOfMemory, "box shape allocation failed"});
    }
}

Result<Shape> PhysicsEngine::CreateSphere(const SphereGeometry& geometry) const
{
    if (!m_Impl || !m_Impl->runtime || !std::isfinite(geometry.radius) || geometry.radius <= 0.0f)
        return std::unexpected(Invalid("invalid sphere radius"));
    try
    {
        JPH::SphereShapeSettings settings(geometry.radius);
        auto impl = ShapeResultToImpl(m_Impl->runtime, settings.Create(), ShapeKind::Sphere, false);
        if (!impl)
            return std::unexpected(impl.error());
        return Shape{std::move(*impl)};
    }
    catch (const std::bad_alloc&)
    {
        return std::unexpected(Error{ErrorCode::OutOfMemory, "sphere shape allocation failed"});
    }
}

Result<Shape> PhysicsEngine::CreateCapsule(const CapsuleGeometry& geometry) const
{
    if (!m_Impl || !m_Impl->runtime || !std::isfinite(geometry.radius) || !std::isfinite(geometry.height)
        || geometry.radius <= 0.0f || geometry.height < 2.0f * geometry.radius)
        return std::unexpected(Invalid("invalid capsule geometry"));
    try
    {
        JPH::CapsuleShapeSettings settings(0.5f * geometry.height - geometry.radius, geometry.radius);
        auto impl = ShapeResultToImpl(m_Impl->runtime, settings.Create(), ShapeKind::Capsule, false);
        if (!impl)
            return std::unexpected(impl.error());
        return Shape{std::move(*impl)};
    }
    catch (const std::bad_alloc&)
    {
        return std::unexpected(Error{ErrorCode::OutOfMemory, "capsule shape allocation failed"});
    }
}

Result<Shape> PhysicsEngine::CreateStaticCompound(std::span<const CompoundChild> children) const
{
    if (!m_Impl || !m_Impl->runtime || children.empty())
        return std::unexpected(Invalid("compound must contain at least one child"));
    std::vector<CompoundMeta> metadata;
    metadata.reserve(children.size());
    bool mustBeStatic = false;
    try
    {
        for (const auto& child : children)
        {
            if (!child.shape.IsValid() || !ValidLocalTransform(child.transform))
                return std::unexpected(Invalid("compound contains an invalid child"));
            metadata.push_back({child.userData});
            mustBeStatic = mustBeStatic || child.shape.m_Impl->mustBeStatic;
        }

        JPH::ShapeSettings::ShapeResult result;
        if (children.size() == 1)
        {
            JPH::RotatedTranslatedShapeSettings settings(
                ToJolt(children[0].transform.position), ToJolt(children[0].transform.rotation),
                children[0].shape.m_Impl->shape);
            result = settings.Create();
        }
        else
        {
            JPH::StaticCompoundShapeSettings settings;
            for (const auto& child : children)
                settings.AddShape(ToJolt(child.transform.position), ToJolt(child.transform.rotation),
                                  child.shape.m_Impl->shape, child.userData);
            result = settings.Create();
        }
        auto impl = ShapeResultToImpl(m_Impl->runtime, std::move(result), ShapeKind::StaticCompound,
                                      mustBeStatic, std::move(metadata));
        if (!impl)
            return std::unexpected(impl.error());
        return Shape{std::move(*impl)};
    }
    catch (const std::bad_alloc&)
    {
        return std::unexpected(Error{ErrorCode::OutOfMemory, "compound shape allocation failed"});
    }
}

Result<Shape> PhysicsEngine::CreateTriangleMesh(const TriangleMeshDesc& desc) const
{
    if (!m_Impl || !m_Impl->runtime || desc.vertices.empty() || desc.indices.empty() || desc.indices.size() % 3 != 0)
        return std::unexpected(Invalid("invalid triangle mesh arrays"));
    try
    {
        JPH::VertexList vertices;
        vertices.reserve(desc.vertices.size());
        for (const auto& vertex : desc.vertices)
        {
            if (!Finite(vertex))
                return std::unexpected(Invalid("triangle mesh contains non-finite vertex"));
            vertices.emplace_back(vertex.x(), vertex.y(), vertex.z());
        }
        JPH::IndexedTriangleList triangles;
        triangles.reserve(desc.indices.size() / 3);
        for (std::size_t i = 0; i < desc.indices.size(); i += 3)
        {
            const auto a = desc.indices[i];
            const auto b = desc.indices[i + 1];
            const auto c = desc.indices[i + 2];
            if (a >= vertices.size() || b >= vertices.size() || c >= vertices.size() || a == b || b == c || a == c)
                return std::unexpected(Invalid("triangle mesh contains an invalid triangle"));
            triangles.emplace_back(a, b, c, 0);
        }
        JPH::MeshShapeSettings settings(std::move(vertices), std::move(triangles));
        auto impl = ShapeResultToImpl(m_Impl->runtime, settings.Create(), ShapeKind::TriangleMesh, true);
        if (!impl)
            return std::unexpected(impl.error());
        return Shape{std::move(*impl)};
    }
    catch (const std::bad_alloc&)
    {
        return std::unexpected(Error{ErrorCode::OutOfMemory, "mesh shape allocation failed"});
    }
}

} // namespace Aether::Physics
