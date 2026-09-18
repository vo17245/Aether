#include "JoltBackend.h"

#include <Jolt/Core/Factory.h>
#include <Jolt/Core/IssueReporting.h>
#include <Jolt/Core/Memory.h>
#include <Jolt/RegisterTypes.h>
#include <cstdarg>
#include <cstdio>
#include <mutex>

namespace Aether::Physics
{
namespace
{
std::mutex gRuntimeMutex;
std::weak_ptr<RuntimeState> gRuntime;
std::once_flag gAllocatorOnce;

void TraceBridge(const char* format, ...)
{
    char buffer[2048]{};
    va_list args;
    va_start(args, format);
    std::vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    if (auto state = gRuntime.lock(); state && state->trace)
        state->trace(buffer);
}

#ifdef JPH_ENABLE_ASSERTS
bool AssertBridge(const char* expression, const char* message, const char* file, JPH::uint line)
{
    char buffer[2048]{};
    std::snprintf(buffer, sizeof(buffer), "%s:%u: (%s) %s", file, line, expression,
                  message != nullptr ? message : "");
    if (auto state = gRuntime.lock(); state && state->trace)
        state->trace(buffer);
    return false;
}
#endif
} // namespace

RuntimeState::~RuntimeState()
{
    std::lock_guard lock(gRuntimeMutex);
    if (JPH::Factory::sInstance != nullptr)
    {
        JPH::UnregisterTypes();
        delete JPH::Factory::sInstance;
        JPH::Factory::sInstance = nullptr;
    }
    JPH::Trace = nullptr;
#ifdef JPH_ENABLE_ASSERTS
    JPH::AssertFailed = nullptr;
#endif
    gRuntime.reset();
}

Result<std::shared_ptr<RuntimeState>> AcquireRuntime(const EngineDesc& desc)
{
    std::lock_guard lock(gRuntimeMutex);
    if (auto current = gRuntime.lock())
    {
        if (desc.trace || current->trace)
            return std::unexpected(Error{ErrorCode::InvalidConfiguration,
                                         "Physics runtime already has a trace callback"});
        return current;
    }

    if (JPH::Factory::sInstance != nullptr)
        return std::unexpected(Error{ErrorCode::BackendFailure, "Jolt Factory is already owned by another runtime"});

    std::call_once(gAllocatorOnce, [] { JPH::RegisterDefaultAllocator(); });
    auto state = std::make_shared<RuntimeState>(desc.trace);
    JPH::Trace = TraceBridge;
#ifdef JPH_ENABLE_ASSERTS
    JPH::AssertFailed = AssertBridge;
#endif
    JPH::Factory::sInstance = new JPH::Factory();
    JPH::RegisterTypes();
    gRuntime = state;
    return state;
}

} // namespace Aether::Physics
