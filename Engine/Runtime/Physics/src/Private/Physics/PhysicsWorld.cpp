#include "JoltBackend.h"
#include "JoltConversion.h"

#include <Jolt/Physics/Body/Body.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Body/BodyLock.h>
#include <Jolt/Physics/Collision/CollideShape.h>
#include <Jolt/Physics/Collision/CastResult.h>
#include <Jolt/Physics/Collision/CollisionCollectorImpl.h>
#include <Jolt/Physics/Collision/NarrowPhaseQuery.h>
#include <Jolt/Physics/Collision/RayCast.h>
#include <Jolt/Physics/Collision/Shape/CompoundShape.h>
#include <Jolt/Physics/Collision/ShapeCast.h>
#include <cmath>

namespace Aether::Physics
{
namespace
{
Error Invalid(std::string message)
{
    return {ErrorCode::InvalidArgument, std::move(message)};
}

Error InvalidHandle()
{
    return {ErrorCode::InvalidHandle, "invalid or stale physics body handle"};
}

bool Finite(const Vec3f& value)
{
    return value.allFinite();
}

bool Finite(const Vec3d& value)
{
    return value.allFinite();
}

bool ValidRotation(const Quatf& rotation)
{
    return rotation.coeffs().allFinite() && std::abs(rotation.norm() - 1.0f) <= 1e-4f;
}

JPH::BodyID ToJoltBodyId(BodyId body)
{
    return JPH::BodyID(body.value);
}

std::uint32_t DirectChildUserData(const std::shared_ptr<const Shape::Impl>& shape,
                                  const JPH::SubShapeID& id)
{
    if (!shape || shape->directChildren.empty())
        return 0;
    if (shape->directChildren.size() == 1)
        return shape->directChildren.front().userData;
    if (shape->shape->GetSubType() != JPH::EShapeSubType::StaticCompound)
        return 0;
    const auto* compound = static_cast<const JPH::CompoundShape*>(shape->shape.GetPtr());
    JPH::SubShapeID remainder;
    const auto index = compound->GetSubShapeIndexFromID(id, remainder);
    return index < shape->directChildren.size() ? shape->directChildren[index].userData : 0;
}

std::shared_ptr<const Shape::Impl> FindShape(const PhysicsWorld::Impl& world, const JPH::BodyID& id)
{
    auto iter = world.bodyShapes.find(id.GetIndexAndSequenceNumber());
    return iter == world.bodyShapes.end() ? nullptr : iter->second;
}

Vec3f ContactNormal(const JPH::Vec3& penetrationAxis, const Vec3f& displacement)
{
    const auto lengthSquared = penetrationAxis.LengthSq();
    if (lengthSquared > 1e-12f)
        return FromJolt(-penetrationAxis.Normalized());
    const auto fallback = -displacement;
    const auto fallbackLength = fallback.norm();
    if (fallbackLength > 1e-6f)
        return (fallback / fallbackLength).eval();
    return Vec3f::UnitY();
}

template <typename F>
Result<void> WithReadBody(const PhysicsWorld::Impl& world, const JPH::BodyID& id, F&& function)
{
    JPH::BodyLockRead lock(world.system.GetBodyLockInterface(), id);
    if (!lock.Succeeded())
        return std::unexpected(InvalidHandle());
    function(lock.GetBody());
    return {};
}
} // namespace

Result<LayerTable> BuildLayerTable(const WorldDesc& desc)
{
    if (desc.maxBodies == 0 || desc.maxBodies > JPH::BodyID::cMaxBodyIndex + 1 || desc.maxBodyPairs == 0
        || desc.maxContactConstraints == 0 || desc.tempAllocatorBytes == 0 || !Finite(desc.gravity))
        return std::unexpected(Error{ErrorCode::InvalidConfiguration, "invalid physics world capacity or gravity"});

    LayerTable table;
    if (desc.layers.empty() || desc.layers.size() > MAX_COLLISION_LAYERS)
        return std::unexpected(Error{ErrorCode::InvalidConfiguration, "physics world must declare 1 to 32 layers"});
    for (const auto& layer : desc.layers)
    {
        if (layer.layer >= MAX_COLLISION_LAYERS || table.declared[layer.layer])
            return std::unexpected(Error{ErrorCode::InvalidConfiguration, "duplicate or out-of-range collision layer"});
        table.declared[layer.layer] = true;
        table.collidesWith[layer.layer] = layer.collidesWith;
    }
    for (CollisionLayer first = 0; first < MAX_COLLISION_LAYERS; ++first)
    {
        if (!table.declared[first])
            continue;
        const auto undeclared = table.collidesWith[first] & ~CollisionMask{0};
        for (CollisionLayer second = 0; second < MAX_COLLISION_LAYERS; ++second)
        {
            if ((undeclared & LayerBit(second)) != 0 && !table.declared[second])
                return std::unexpected(Error{ErrorCode::InvalidConfiguration,
                                             "collision layer references an undeclared layer"});
            if (table.declared[second] && (table.collidesWith[first] & LayerBit(second)) != 0
                && (table.collidesWith[second] & LayerBit(first)) == 0)
                return std::unexpected(Error{ErrorCode::InvalidConfiguration,
                                             "collision layer matrix must be symmetric"});
        }
    }
    return table;
}

JPH::ObjectLayer EncodeObjectLayer(CollisionLayer layer, MotionType motion)
{
    return static_cast<JPH::ObjectLayer>(static_cast<std::uint16_t>(layer) * 2u
                                         + (motion == MotionType::Static ? 0u : 1u));
}

CollisionLayer DecodeLogicalLayer(JPH::ObjectLayer layer)
{
    return static_cast<CollisionLayer>(layer / 2u);
}

bool IsMovingObjectLayer(JPH::ObjectLayer layer)
{
    return (layer & 1u) != 0;
}

bool QueryBodyFilter::ShouldCollideLocked(const JPH::Body& body) const
{
    return m_Filter.includeSensors || !body.IsSensor();
}

Result<void> PhysicsWorld::Impl::ValidateOwner() const
{
    if (!OnOwnerThread())
        return std::unexpected(Error{ErrorCode::BackendFailure, "physics world accessed from a non-owner thread"});
    return {};
}

Result<JPH::BodyID> PhysicsWorld::Impl::ToJoltBody(BodyId body) const
{
    if (!body.IsValid())
        return std::unexpected(InvalidHandle());
    const JPH::BodyID result = ToJoltBodyId(body);
    if (!system.GetBodyInterface().IsAdded(result))
        return std::unexpected(InvalidHandle());
    return result;
}

PhysicsWorld::Impl::~Impl()
{
    auto& bodies = system.GetBodyInterface();
    for (const auto& [value, shape] : bodyShapes)
    {
        const JPH::BodyID id(value);
        if (bodies.IsAdded(id))
            bodies.RemoveBody(id);
        bodies.DestroyBody(id);
    }
    bodyShapes.clear();
}

PhysicsWorld::PhysicsWorld(std::unique_ptr<Impl> impl) : m_Impl(std::move(impl)) {}
PhysicsWorld::PhysicsWorld(PhysicsWorld&&) noexcept = default;
PhysicsWorld& PhysicsWorld::operator=(PhysicsWorld&&) noexcept = default;
PhysicsWorld::~PhysicsWorld() = default;

Result<BodyId> PhysicsWorld::CreateBody(const BodyDesc& desc)
{
    if (!m_Impl)
        return std::unexpected(Error{ErrorCode::BackendFailure, "physics world is not initialized"});
    if (auto owner = m_Impl->ValidateOwner(); !owner)
        return std::unexpected(owner.error());
    if (!desc.shape.IsValid() || !Finite(desc.transform.position) || !ValidRotation(desc.transform.rotation)
        || !Finite(desc.linearVelocity) || !Finite(desc.angularVelocity) || desc.layer >= MAX_COLLISION_LAYERS
        || !m_Impl->layers.IsDeclared(desc.layer))
        return std::unexpected(Invalid("invalid body description"));
    if (desc.shape.m_Impl->mustBeStatic && desc.motionType != MotionType::Static)
        return std::unexpected(Invalid("the selected shape can only be used by a static body"));
    if (desc.motionType == MotionType::Static
        && (desc.linearVelocity.squaredNorm() > 0.0f || desc.angularVelocity.squaredNorm() > 0.0f))
        return std::unexpected(Invalid("static body cannot have initial velocity"));
    if (desc.motionType == MotionType::Dynamic && (!std::isfinite(desc.mass) || desc.mass <= 0.0f))
        return std::unexpected(Invalid("dynamic body mass must be finite and positive"));
    if (!std::isfinite(desc.friction) || !std::isfinite(desc.restitution) || !std::isfinite(desc.linearDamping)
        || !std::isfinite(desc.angularDamping) || !std::isfinite(desc.gravityFactor))
        return std::unexpected(Invalid("body material values must be finite"));

    try
    {
        const auto motion = desc.motionType == MotionType::Static
            ? JPH::EMotionType::Static
            : desc.motionType == MotionType::Kinematic ? JPH::EMotionType::Kinematic : JPH::EMotionType::Dynamic;
        JPH::BodyCreationSettings settings(desc.shape.m_Impl->shape, ToJoltPosition(desc.transform.position),
                                            ToJolt(desc.transform.rotation), motion,
                                            EncodeObjectLayer(desc.layer, desc.motionType));
        settings.mUserData = desc.userData;
        settings.mLinearVelocity = ToJolt(desc.linearVelocity);
        settings.mAngularVelocity = ToJolt(desc.angularVelocity);
        settings.mMotionQuality = desc.motionQuality == MotionQuality::LinearCast
            ? JPH::EMotionQuality::LinearCast : JPH::EMotionQuality::Discrete;
        settings.mIsSensor = desc.isSensor;
        settings.mAllowSleeping = desc.allowSleeping;
        settings.mFriction = desc.friction;
        settings.mRestitution = desc.restitution;
        settings.mLinearDamping = desc.linearDamping;
        settings.mAngularDamping = desc.angularDamping;
        settings.mGravityFactor = desc.gravityFactor;
        if (desc.motionType == MotionType::Dynamic)
        {
            settings.mOverrideMassProperties = JPH::EOverrideMassProperties::CalculateInertia;
            settings.mMassPropertiesOverride.mMass = desc.mass;
        }
        const auto activation = desc.startActive ? JPH::EActivation::Activate : JPH::EActivation::DontActivate;
        const auto id = m_Impl->system.GetBodyInterface().CreateAndAddBody(settings, activation);
        if (id.IsInvalid())
            return std::unexpected(Error{ErrorCode::CapacityExceeded, "Jolt could not allocate a body"});
        m_Impl->bodyShapes.emplace(id.GetIndexAndSequenceNumber(), desc.shape.m_Impl);
        return BodyId{id.GetIndexAndSequenceNumber()};
    }
    catch (const std::bad_alloc&)
    {
        return std::unexpected(Error{ErrorCode::OutOfMemory, "body allocation failed"});
    }
}

Result<void> PhysicsWorld::DestroyBody(BodyId body)
{
    if (!m_Impl)
        return std::unexpected(Error{ErrorCode::BackendFailure, "physics world is not initialized"});
    if (auto owner = m_Impl->ValidateOwner(); !owner)
        return std::unexpected(owner.error());
    auto id = m_Impl->ToJoltBody(body);
    if (!id)
        return std::unexpected(id.error());
    auto& bodies = m_Impl->system.GetBodyInterface();
    bodies.RemoveBody(*id);
    bodies.DestroyBody(*id);
    m_Impl->bodyShapes.erase(body.value);
    return {};
}

bool PhysicsWorld::IsBodyValid(BodyId body) const
{
    return m_Impl && m_Impl->OnOwnerThread() && body.IsValid()
        && m_Impl->system.GetBodyInterface().IsAdded(ToJoltBodyId(body));
}

Result<WorldTransform> PhysicsWorld::GetTransform(BodyId body) const
{
    if (!m_Impl)
        return std::unexpected(Error{ErrorCode::BackendFailure, "physics world is not initialized"});
    if (auto owner = m_Impl->ValidateOwner(); !owner)
        return std::unexpected(owner.error());
    auto id = m_Impl->ToJoltBody(body);
    if (!id)
        return std::unexpected(id.error());
    JPH::RVec3 position;
    JPH::Quat rotation;
    m_Impl->system.GetBodyInterface().GetPositionAndRotation(*id, position, rotation);
    return WorldTransform{FromJoltPosition(position), FromJolt(rotation)};
}

Result<void> PhysicsWorld::SetTransform(BodyId body, const WorldTransform& transform, bool activate)
{
    if (!m_Impl)
        return std::unexpected(Error{ErrorCode::BackendFailure, "physics world is not initialized"});
    if (auto owner = m_Impl->ValidateOwner(); !owner)
        return std::unexpected(owner.error());
    if (!Finite(transform.position) || !ValidRotation(transform.rotation))
        return std::unexpected(Invalid("invalid body transform"));
    auto id = m_Impl->ToJoltBody(body);
    if (!id)
        return std::unexpected(id.error());
    m_Impl->system.GetBodyInterface().SetPositionAndRotation(*id, ToJoltPosition(transform.position),
                                                             ToJolt(transform.rotation),
                                                             activate ? JPH::EActivation::Activate : JPH::EActivation::DontActivate);
    return {};
}

Result<Vec3f> PhysicsWorld::GetLinearVelocity(BodyId body) const
{
    if (!m_Impl)
        return std::unexpected(Error{ErrorCode::BackendFailure, "physics world is not initialized"});
    if (auto owner = m_Impl->ValidateOwner(); !owner)
        return std::unexpected(owner.error());
    auto id = m_Impl->ToJoltBody(body);
    if (!id)
        return std::unexpected(id.error());
    return FromJolt(m_Impl->system.GetBodyInterface().GetLinearVelocity(*id));
}

Result<void> PhysicsWorld::SetLinearVelocity(BodyId body, const Vec3f& velocity)
{
    if (!m_Impl)
        return std::unexpected(Error{ErrorCode::BackendFailure, "physics world is not initialized"});
    if (auto owner = m_Impl->ValidateOwner(); !owner)
        return std::unexpected(owner.error());
    if (!Finite(velocity))
        return std::unexpected(Invalid("invalid linear velocity"));
    auto id = m_Impl->ToJoltBody(body);
    if (!id)
        return std::unexpected(id.error());
    JPH::BodyLockRead lock(m_Impl->system.GetBodyLockInterface(), *id);
    if (!lock.Succeeded())
        return std::unexpected(InvalidHandle());
    if (lock.GetBody().GetMotionType() == JPH::EMotionType::Static)
        return std::unexpected(Invalid("static body cannot have velocity"));
    m_Impl->system.GetBodyInterface().SetLinearVelocity(*id, ToJolt(velocity));
    return {};
}

Result<void> PhysicsWorld::AddForce(BodyId body, const Vec3f& force)
{
    if (!m_Impl)
        return std::unexpected(Error{ErrorCode::BackendFailure, "physics world is not initialized"});
    if (auto owner = m_Impl->ValidateOwner(); !owner)
        return std::unexpected(owner.error());
    if (!Finite(force))
        return std::unexpected(Invalid("invalid force"));
    auto id = m_Impl->ToJoltBody(body);
    if (!id)
        return std::unexpected(id.error());
    JPH::BodyLockRead lock(m_Impl->system.GetBodyLockInterface(), *id);
    if (!lock.Succeeded() || lock.GetBody().GetMotionType() != JPH::EMotionType::Dynamic)
        return std::unexpected(Invalid("only dynamic bodies accept force"));
    m_Impl->system.GetBodyInterface().AddForce(*id, ToJolt(force));
    return {};
}

Result<void> PhysicsWorld::AddImpulse(BodyId body, const Vec3f& impulse)
{
    if (!m_Impl)
        return std::unexpected(Error{ErrorCode::BackendFailure, "physics world is not initialized"});
    if (auto owner = m_Impl->ValidateOwner(); !owner)
        return std::unexpected(owner.error());
    if (!Finite(impulse))
        return std::unexpected(Invalid("invalid impulse"));
    auto id = m_Impl->ToJoltBody(body);
    if (!id)
        return std::unexpected(id.error());
    JPH::BodyLockRead lock(m_Impl->system.GetBodyLockInterface(), *id);
    if (!lock.Succeeded() || lock.GetBody().GetMotionType() != JPH::EMotionType::Dynamic)
        return std::unexpected(Invalid("only dynamic bodies accept impulse"));
    m_Impl->system.GetBodyInterface().AddImpulse(*id, ToJolt(impulse));
    return {};
}

Result<void> PhysicsWorld::MoveKinematic(BodyId body, const WorldTransform& target, float deltaTime)
{
    if (!m_Impl)
        return std::unexpected(Error{ErrorCode::BackendFailure, "physics world is not initialized"});
    if (auto owner = m_Impl->ValidateOwner(); !owner)
        return std::unexpected(owner.error());
    if (!Finite(target.position) || !ValidRotation(target.rotation) || !std::isfinite(deltaTime) || deltaTime <= 0.0f)
        return std::unexpected(Invalid("invalid kinematic movement"));
    auto id = m_Impl->ToJoltBody(body);
    if (!id)
        return std::unexpected(id.error());
    JPH::BodyLockRead lock(m_Impl->system.GetBodyLockInterface(), *id);
    if (!lock.Succeeded() || lock.GetBody().GetMotionType() != JPH::EMotionType::Kinematic)
        return std::unexpected(Invalid("MoveKinematic requires a kinematic body"));
    m_Impl->system.GetBodyInterface().MoveKinematic(*id, ToJoltPosition(target.position), ToJolt(target.rotation), deltaTime);
    return {};
}

Result<UserData> PhysicsWorld::GetUserData(BodyId body) const
{
    if (!m_Impl)
        return std::unexpected(Error{ErrorCode::BackendFailure, "physics world is not initialized"});
    if (auto owner = m_Impl->ValidateOwner(); !owner)
        return std::unexpected(owner.error());
    auto id = m_Impl->ToJoltBody(body);
    if (!id)
        return std::unexpected(id.error());
    UserData value = 0;
    auto result = WithReadBody(*m_Impl, *id, [&](const JPH::Body& native) { value = native.GetUserData(); });
    if (!result)
        return std::unexpected(result.error());
    return value;
}

Result<void> PhysicsWorld::Step(float deltaTime, std::uint32_t collisionSteps)
{
    if (!m_Impl)
        return std::unexpected(Error{ErrorCode::BackendFailure, "physics world is not initialized"});
    if (auto owner = m_Impl->ValidateOwner(); !owner)
        return std::unexpected(owner.error());
    if (!std::isfinite(deltaTime) || deltaTime <= 0.0f || collisionSteps == 0)
        return std::unexpected(Invalid("invalid physics step"));
    const auto error = m_Impl->system.Update(static_cast<float>(deltaTime), static_cast<int>(collisionSteps),
                                             m_Impl->tempAllocator.get(), m_Impl->jobSystem.get());
    if (error != JPH::EPhysicsUpdateError::None)
        return std::unexpected(Error{ErrorCode::BackendFailure, "Jolt physics update failed"});
    return {};
}

void PhysicsWorld::OptimizeBroadPhase()
{
    if (m_Impl && m_Impl->OnOwnerThread())
        m_Impl->system.OptimizeBroadPhase();
}

Result<std::optional<RayCastHit>> PhysicsWorld::CastRayClosest(const RayCast& cast, const QueryFilter& filter) const
{
    if (!m_Impl)
        return std::unexpected(Error{ErrorCode::BackendFailure, "physics world is not initialized"});
    if (auto owner = m_Impl->ValidateOwner(); !owner)
        return std::unexpected(owner.error());
    if (!Finite(cast.origin) || !Finite(cast.displacement) || cast.displacement.squaredNorm() <= 1e-12f)
        return std::unexpected(Invalid("ray cast requires a finite non-zero displacement"));
    const auto ignored = filter.ignoredBody ? ToJoltBodyId(*filter.ignoredBody) : JPH::BodyID();
    QueryObjectLayerFilter objectFilter(filter.layers);
    AllBroadPhaseLayerFilter broadFilter;
    QueryBodyFilter bodyFilter(filter, ignored);
    JPH::RRayCast ray(ToJoltPosition(cast.origin), ToJolt(cast.displacement));
    JPH::RayCastResult result;
    if (!m_Impl->system.GetNarrowPhaseQuery().CastRay(ray, result, broadFilter, objectFilter, bodyFilter))
        return std::optional<RayCastHit>{};

    RayCastHit hit;
    hit.body = BodyId{result.mBodyID.GetIndexAndSequenceNumber()};
    hit.fraction = result.mFraction;
    hit.distance = cast.displacement.norm() * result.mFraction;
    const auto point = ray.GetPointOnRay(result.mFraction);
    hit.point = FromJoltPosition(point);
    auto shape = FindShape(*m_Impl, result.mBodyID);
    hit.subShapeUserData = DirectChildUserData(shape, result.mSubShapeID2);
    auto bodyResult = WithReadBody(*m_Impl, result.mBodyID, [&](const JPH::Body& native) {
        hit.bodyUserData = native.GetUserData();
        hit.normal = FromJolt(native.GetWorldSpaceSurfaceNormal(result.mSubShapeID2, point));
    });
    if (!bodyResult)
        return std::unexpected(bodyResult.error());
    return std::optional<RayCastHit>{hit};
}

Result<std::optional<ShapeCastHit>> PhysicsWorld::CastShapeClosest(const ShapeCast& cast,
                                                                    const ShapeCastOptions& options) const
{
    if (!m_Impl)
        return std::unexpected(Error{ErrorCode::BackendFailure, "physics world is not initialized"});
    if (auto owner = m_Impl->ValidateOwner(); !owner)
        return std::unexpected(owner.error());
    if (!cast.shape.IsValid() || !Finite(cast.start.position) || !ValidRotation(cast.start.rotation)
        || !Finite(cast.displacement) || cast.displacement.squaredNorm() <= 1e-12f)
        return std::unexpected(Invalid("shape cast requires a valid shape and finite non-zero displacement"));
    const auto ignored = options.filter.ignoredBody ? ToJoltBodyId(*options.filter.ignoredBody) : JPH::BodyID();
    QueryObjectLayerFilter objectFilter(options.filter.layers);
    AllBroadPhaseLayerFilter broadFilter;
    QueryBodyFilter bodyFilter(options.filter, ignored);
    const auto transform = JPH::RMat44::sRotationTranslation(ToJolt(cast.start.rotation),
                                                               ToJoltPosition(cast.start.position));
    JPH::RShapeCast shapeCast = JPH::RShapeCast::sFromWorldTransform(
        cast.shape.m_Impl->shape, JPH::Vec3::sOne(), transform, ToJolt(cast.displacement));
    JPH::ShapeCastSettings settings;
    settings.mUseShrunkenShapeAndConvexRadius = options.useShrunkenShapeAndConvexRadius;
    settings.mReturnDeepestPoint = options.returnDeepestPointForInitialOverlap;
    settings.mBackFaceModeConvex = JPH::EBackFaceMode::CollideWithBackFaces;
    JPH::ClosestHitCollisionCollector<JPH::CastShapeCollector> collector;
    m_Impl->system.GetNarrowPhaseQuery().CastShape(shapeCast, settings, ToJoltPosition(cast.start.position), collector,
                                                   broadFilter, objectFilter, bodyFilter);
    if (!collector.HadHit())
        return std::optional<ShapeCastHit>{};

    const auto& result = collector.mHit;
    ShapeCastHit hit;
    hit.body = BodyId{result.mBodyID2.GetIndexAndSequenceNumber()};
    hit.fraction = result.mFraction;
    hit.distance = cast.displacement.norm() * result.mFraction;
    hit.castOriginAtHit = cast.start.position + cast.displacement.cast<double>() * result.mFraction;
    hit.pointOnTarget = cast.start.position + FromJolt(result.mContactPointOn2).cast<double>();
    hit.normal = ContactNormal(result.mPenetrationAxis, cast.displacement);
    hit.penetrationDepth = result.mPenetrationDepth;
    hit.startedOverlapping = result.mFraction <= 0.0f;
    auto shape = FindShape(*m_Impl, result.mBodyID2);
    hit.subShapeUserData = DirectChildUserData(shape, result.mSubShapeID2);
    auto bodyResult = WithReadBody(*m_Impl, result.mBodyID2,
                                   [&](const JPH::Body& native) { hit.bodyUserData = native.GetUserData(); });
    if (!bodyResult)
        return std::unexpected(bodyResult.error());
    return std::optional<ShapeCastHit>{hit};
}

Result<bool> PhysicsWorld::CastShapeAny(const ShapeCast& cast, const ShapeCastOptions& options) const
{
    if (!m_Impl)
        return std::unexpected(Error{ErrorCode::BackendFailure, "physics world is not initialized"});
    if (auto owner = m_Impl->ValidateOwner(); !owner)
        return std::unexpected(owner.error());
    if (!cast.shape.IsValid() || !Finite(cast.start.position) || !ValidRotation(cast.start.rotation)
        || !Finite(cast.displacement) || cast.displacement.squaredNorm() <= 1e-12f)
        return std::unexpected(Invalid("shape cast requires a valid shape and finite non-zero displacement"));
    const auto ignored = options.filter.ignoredBody ? ToJoltBodyId(*options.filter.ignoredBody) : JPH::BodyID();
    QueryObjectLayerFilter objectFilter(options.filter.layers);
    AllBroadPhaseLayerFilter broadFilter;
    QueryBodyFilter bodyFilter(options.filter, ignored);
    const auto transform = JPH::RMat44::sRotationTranslation(ToJolt(cast.start.rotation),
                                                               ToJoltPosition(cast.start.position));
    JPH::RShapeCast shapeCast = JPH::RShapeCast::sFromWorldTransform(
        cast.shape.m_Impl->shape, JPH::Vec3::sOne(), transform, ToJolt(cast.displacement));
    JPH::ShapeCastSettings settings;
    settings.mUseShrunkenShapeAndConvexRadius = options.useShrunkenShapeAndConvexRadius;
    settings.mReturnDeepestPoint = options.returnDeepestPointForInitialOverlap;
    settings.mBackFaceModeConvex = JPH::EBackFaceMode::CollideWithBackFaces;
    JPH::AnyHitCollisionCollector<JPH::CastShapeCollector> collector;
    m_Impl->system.GetNarrowPhaseQuery().CastShape(shapeCast, settings, ToJoltPosition(cast.start.position), collector,
                                                   broadFilter, objectFilter, bodyFilter);
    return collector.HadHit();
}

Result<std::vector<OverlapHit>> PhysicsWorld::OverlapShape(const ShapeOverlap& overlap,
                                                           const QueryFilter& filter) const
{
    if (!m_Impl)
        return std::unexpected(Error{ErrorCode::BackendFailure, "physics world is not initialized"});
    if (auto owner = m_Impl->ValidateOwner(); !owner)
        return std::unexpected(owner.error());
    if (!overlap.shape.IsValid() || !Finite(overlap.transform.position) || !ValidRotation(overlap.transform.rotation))
        return std::unexpected(Invalid("overlap requires a valid shape and transform"));
    try
    {
        const auto ignored = filter.ignoredBody ? ToJoltBodyId(*filter.ignoredBody) : JPH::BodyID();
        QueryObjectLayerFilter objectFilter(filter.layers);
        AllBroadPhaseLayerFilter broadFilter;
        QueryBodyFilter bodyFilter(filter, ignored);
        const auto transform = JPH::RMat44::sRotationTranslation(ToJolt(overlap.transform.rotation),
                                                                   ToJoltPosition(overlap.transform.position));
        JPH::CollideShapeSettings settings;
        JPH::AllHitCollisionCollector<JPH::CollideShapeCollector> collector;
        m_Impl->system.GetNarrowPhaseQuery().CollideShape(overlap.shape.m_Impl->shape, JPH::Vec3::sOne(), transform,
                                                           settings, ToJoltPosition(overlap.transform.position), collector,
                                                           broadFilter, objectFilter, bodyFilter);
        std::vector<OverlapHit> hits;
        hits.reserve(collector.mHits.size());
        for (const auto& result : collector.mHits)
        {
            OverlapHit hit;
            hit.body = BodyId{result.mBodyID2.GetIndexAndSequenceNumber()};
            hit.penetrationDepth = std::max(0.0f, result.mPenetrationDepth);
            hit.separationDirection = ContactNormal(result.mPenetrationAxis, Vec3f::Zero());
            auto shape = FindShape(*m_Impl, result.mBodyID2);
            hit.subShapeUserData = DirectChildUserData(shape, result.mSubShapeID2);
            auto bodyResult = WithReadBody(*m_Impl, result.mBodyID2,
                                           [&](const JPH::Body& native) { hit.bodyUserData = native.GetUserData(); });
            if (!bodyResult)
                return std::unexpected(bodyResult.error());
            hits.push_back(hit);
        }
        return hits;
    }
    catch (const std::bad_alloc&)
    {
        return std::unexpected(Error{ErrorCode::OutOfMemory, "overlap result allocation failed"});
    }
}

} // namespace Aether::Physics
