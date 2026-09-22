#pragma once

#include <cstdint>

namespace Aether
{
using WindowId = std::uint32_t;

struct PixelExtent
{
    std::uint32_t width = 0;
    std::uint32_t height = 0;

    friend bool operator==(const PixelExtent&, const PixelExtent&) = default;
};

struct WindowState
{
    PixelExtent pixelExtent;
    bool minimized = false;
    std::uint64_t version = 0;
};
} // namespace Aether
