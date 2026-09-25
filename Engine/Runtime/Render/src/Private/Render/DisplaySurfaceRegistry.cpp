#include <Render/DisplaySurfaceRegistry.h>

#include <limits>

namespace Aether::Render
{
namespace Detail
{
struct DisplaySurfaceSlot
{
    DisplaySurfaceToken token;
    DisplaySurfaceExtent extent;
    bool ready = false;
    bool retiring = false;
    std::uint64_t lastSubmissionSerial = 0;
};
}

DisplaySurfaceToken DisplaySurfaceLease::Token() const noexcept
{
    return m_Slot ? m_Slot->token : DisplaySurfaceToken{};
}

DisplaySurfaceExtent DisplaySurfaceLease::Extent() const noexcept
{
    return m_Slot ? m_Slot->extent : DisplaySurfaceExtent{};
}

DisplaySurfaceToken DisplaySurfaceRegistry::Create()
{
    if (m_NextId == 0 || m_NextId == std::numeric_limits<std::uint64_t>::max()) return {};
    auto slot = std::make_shared<Detail::DisplaySurfaceSlot>();
    slot->token = {m_NextId++, 1};
    m_Current.emplace(slot->token.id, slot);
    return slot->token;
}

std::optional<DisplaySurfaceToken> DisplaySurfaceRegistry::Replace(DisplaySurfaceToken current,
    std::uint64_t lastSubmissionSerial)
{
    const auto found = m_Current.find(current.id);
    if (!current.IsValid() || found == m_Current.end() || found->second->token != current
        || current.generation == std::numeric_limits<std::uint32_t>::max()) return std::nullopt;
    auto replacement = std::make_shared<Detail::DisplaySurfaceSlot>();
    replacement->token = {current.id, current.generation + 1};
    m_Retired.push_back(found->second);
    found->second->retiring = true;
    found->second->lastSubmissionSerial = lastSubmissionSerial;
    found->second = replacement;
    Reclaim();
    return replacement->token;
}

bool DisplaySurfaceRegistry::Publish(DisplaySurfaceToken token, DisplaySurfaceExtent extent)
{
    const auto found = m_Current.find(token.id);
    if (!token.IsValid() || found == m_Current.end() || found->second->token != token
        || extent.width == 0 || extent.height == 0 || found->second->retiring) return false;
    found->second->extent = extent;
    found->second->ready = true;
    return true;
}

std::optional<DisplaySurfaceLease> DisplaySurfaceRegistry::Acquire(DisplaySurfaceToken token) const
{
    const auto found = m_Current.find(token.id);
    if (!token.IsValid() || found == m_Current.end() || found->second->token != token || !found->second->ready)
        return std::nullopt;
    return DisplaySurfaceLease(found->second);
}

bool DisplaySurfaceRegistry::Retire(DisplaySurfaceToken token, std::uint64_t lastSubmissionSerial)
{
    const auto found = m_Current.find(token.id);
    if (!token.IsValid() || found == m_Current.end() || found->second->token != token) return false;
    m_Retired.push_back(found->second);
    found->second->retiring = true;
    found->second->lastSubmissionSerial = lastSubmissionSerial;
    m_Current.erase(found);
    Reclaim();
    return true;
}

void DisplaySurfaceRegistry::CompleteThrough(std::uint64_t submissionSerial)
{
    if (submissionSerial > m_CompletedSerial) m_CompletedSerial = submissionSerial;
    Reclaim();
}

void DisplaySurfaceRegistry::Reclaim()
{
    std::erase_if(m_Retired, [&](const auto& slot) {
        return slot->lastSubmissionSerial <= m_CompletedSerial && slot.use_count() == 1;
    });
}
}
