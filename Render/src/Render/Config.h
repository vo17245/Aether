#pragma once
#include "vulkan/vulkan.h"
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
    static int MaxFramesInFlight;
    static constexpr const inline int InFlightFrameResourceSlots = 3;
};
} // namespace Render
} // namespace Aether
#ifndef AETHER_RENDER_GRAPH_ENABLE_TAG
#define AETHER_RENDER_GRAPH_ENABLE_TAG 1
#endif