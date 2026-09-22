#pragma once

#include <cstdint>
#include <limits>

namespace Aether::Render
{
template <typename Tag = void>
struct RenderResourceHandle
{
    std::uint32_t index = std::numeric_limits<std::uint32_t>::max();
    std::uint32_t generation = 0;

    bool IsValid() const noexcept
    {
        return index != std::numeric_limits<std::uint32_t>::max() && generation != 0;
    }

    explicit operator bool() const noexcept { return IsValid(); }
    friend bool operator==(const RenderResourceHandle&, const RenderResourceHandle&) = default;
};
} // namespace Aether::Render
