#pragma once
#include <Render/Vulkan/Fence.h>
#include <cassert>
namespace Aether
{
enum class DevicePipelineSyncStage
{
    AllGraphics,
};
inline constexpr VkPipelineStageFlags DevicePipelineSyncStageToVk(DevicePipelineSyncStage stage)
{
    switch (stage)
    {
    case DevicePipelineSyncStage::AllGraphics:
        return VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT;
    default:
        assert(false && "Not implemented");
        return VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT;
    }
}
} // namespace Aether