#pragma once

#include <Core/DisplaySurfaceToken.h>

#include <cstdint>
#include <memory>
#include <optional>
#include <unordered_map>
#include <vector>

namespace Aether::Render
{
namespace Detail { struct DisplaySurfaceSlot; }

struct DisplaySurfaceExtent
{
    std::uint32_t width = 0;
    std::uint32_t height = 0;
};

class DisplaySurfaceLease
{
public:
    DisplaySurfaceLease() = default;
    DisplaySurfaceToken Token() const noexcept;
    DisplaySurfaceExtent Extent() const noexcept;
    explicit operator bool() const noexcept { return static_cast<bool>(m_Slot); }
private:
    explicit DisplaySurfaceLease(std::shared_ptr<const Detail::DisplaySurfaceSlot> slot) : m_Slot(std::move(slot)) {}
    std::shared_ptr<const Detail::DisplaySurfaceSlot> m_Slot;
    friend class DisplaySurfaceRegistry;
};

// Render-thread-owned logical surface table. Leases keep an old generation
// alive while submitted frames still refer to it; GPU completion is reported
// separately from CPU packet consumption.
class DisplaySurfaceRegistry
{
public:
    DisplaySurfaceToken Create();
    std::optional<DisplaySurfaceToken> Replace(DisplaySurfaceToken current, std::uint64_t lastSubmissionSerial);
    bool Publish(DisplaySurfaceToken token, DisplaySurfaceExtent extent);
    std::optional<DisplaySurfaceLease> Acquire(DisplaySurfaceToken token) const;
    bool Retire(DisplaySurfaceToken token, std::uint64_t lastSubmissionSerial);
    void CompleteThrough(std::uint64_t submissionSerial);
    std::size_t PendingRetirements() const noexcept { return m_Retired.size(); }
    std::uint64_t CompletedSerial() const noexcept { return m_CompletedSerial; }
private:
    std::uint64_t m_NextId = 1;
    std::uint64_t m_CompletedSerial = 0;
    std::unordered_map<std::uint64_t, std::shared_ptr<Detail::DisplaySurfaceSlot>> m_Current;
    std::vector<std::shared_ptr<Detail::DisplaySurfaceSlot>> m_Retired;
    void Reclaim();
};
}
