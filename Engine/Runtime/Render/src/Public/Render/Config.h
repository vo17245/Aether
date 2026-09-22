#pragma once
#include "vulkan/vulkan.h"
#include <cstdint>
namespace Aether
{
namespace Render
{
enum class Api
{
    Vulkan,
    OpenGL,
    DirectX
};
class Config
{
public:
    // setup when application start, and never change
    static Api RenderApi;
    static int VulkanApiVersion;
    static int VulkanApiVersionNumber;
    static constexpr const inline int InFlightFrameResourceSlots = 3;
    static constexpr const inline std::uint32_t MaxFramesInFlight = 2;
    static_assert(MaxFramesInFlight >= 1 && MaxFramesInFlight <= InFlightFrameResourceSlots);
};
} // namespace Render
} // namespace Aether
