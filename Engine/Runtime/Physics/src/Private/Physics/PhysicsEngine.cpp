#include "JoltBackend.h"
#include "JoltConversion.h"
#include <algorithm>

namespace Aether::Physics
{

PhysicsEngine::PhysicsEngine(std::unique_ptr<Impl> impl) : m_Impl(std::move(impl)) {}
PhysicsEngine::PhysicsEngine(PhysicsEngine&&) noexcept = default;
PhysicsEngine& PhysicsEngine::operator=(PhysicsEngine&&) noexcept = default;
PhysicsEngine::~PhysicsEngine() = default;

Result<std::unique_ptr<PhysicsEngine>> PhysicsEngine::Create(const EngineDesc& desc)
{
    auto runtime = AcquireRuntime(desc);
    if (!runtime)
        return std::unexpected(runtime.error());
    try
    {
        auto impl = std::make_unique<Impl>();
        impl->runtime = std::move(*runtime);
        return std::unique_ptr<PhysicsEngine>(new PhysicsEngine(std::move(impl)));
    }
    catch (const std::bad_alloc&)
    {
        return std::unexpected(Error{ErrorCode::OutOfMemory, "physics engine allocation failed"});
    }
}

Result<std::unique_ptr<PhysicsWorld>> PhysicsEngine::CreateWorld(const WorldDesc& desc) const
{
    if (!m_Impl || !m_Impl->runtime)
        return std::unexpected(Error{ErrorCode::BackendFailure, "physics engine is not initialized"});
    auto layers = BuildLayerTable(desc);
    if (!layers)
        return std::unexpected(layers.error());
    try
    {
        auto impl = std::make_unique<PhysicsWorld::Impl>(m_Impl->runtime, desc, *layers);
        impl->tempAllocator = std::make_unique<JPH::TempAllocatorImpl>(desc.tempAllocatorBytes);
        const auto workers = desc.workerThreads == 0
            ? std::max(1u, std::thread::hardware_concurrency() > 1 ? std::thread::hardware_concurrency() - 1 : 1u)
            : desc.workerThreads;
        impl->jobSystem = std::make_unique<JPH::JobSystemThreadPool>(JPH::cMaxPhysicsJobs, JPH::cMaxPhysicsBarriers, workers);
        impl->system.Init(desc.maxBodies, desc.numBodyMutexes, desc.maxBodyPairs, desc.maxContactConstraints,
                          impl->broadPhaseLayers, impl->objectVsBroadPhaseFilter, impl->objectLayerPairFilter);
        impl->system.SetGravity(ToJolt(desc.gravity));
        return std::unique_ptr<PhysicsWorld>(new PhysicsWorld(std::move(impl)));
    }
    catch (const std::bad_alloc&)
    {
        return std::unexpected(Error{ErrorCode::OutOfMemory, "physics world allocation failed"});
    }
}

} // namespace Aether::Physics
