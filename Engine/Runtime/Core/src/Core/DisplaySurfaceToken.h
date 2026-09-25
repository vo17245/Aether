#pragma once

#include <cstdint>

namespace Aether
{
// A renderer-owned image reference. Generation changes when the surface is
// replaced, so frames already in flight can keep the previous slot alive.
struct DisplaySurfaceToken
{
    std::uint64_t id = 0;
    std::uint32_t generation = 0;
    constexpr bool IsValid() const noexcept { return id != 0 && generation != 0; }
    friend constexpr bool operator==(DisplaySurfaceToken, DisplaySurfaceToken) = default;
};
}
