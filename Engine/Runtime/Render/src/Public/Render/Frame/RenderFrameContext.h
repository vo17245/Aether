#pragma once

#include <cstdint>
#include <optional>

namespace Aether::Render
{
using CpuFrameId = std::uint64_t;
using SubmissionSerial = std::uint64_t;

struct RenderFrameContext
{
    std::uint32_t frameSlot = 0;
    std::optional<CpuFrameId> cpuFrameId;
    SubmissionSerial completedSerial = 0;
};
} // namespace Aether::Render
