#pragma once

#include <cstdint>
#include <optional>

namespace Aether::Render
{
}

namespace Aether
{
class InFlightResourceAllocator;
class PendingUploadList;
namespace RenderGraph
{
class ResourceArena;
}
}

namespace Aether::Render
{
using CpuFrameId = std::uint64_t;
using SubmissionSerial = std::uint64_t;

// Services owned by a render window and valid for the duration of every
// RenderFeature callback.  They deliberately expose no Window/SDL state.
struct RenderFeatureServices
{
    InFlightResourceAllocator& resources;
    RenderGraph::ResourceArena& arena;
    PendingUploadList& uploads;
};

struct RenderFrameContext
{
    std::uint32_t frameSlot = 0;
    std::optional<CpuFrameId> cpuFrameId;
    SubmissionSerial completedSerial = 0;
    RenderFeatureServices* services = nullptr;
};
} // namespace Aether::Render
