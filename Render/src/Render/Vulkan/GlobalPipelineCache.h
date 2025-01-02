#pragma once
#include "vulkan/vulkan.h"
#include "vulkan/vulkan_core.h"
#include "GlobalRenderContext.h"
namespace Aether {
namespace vk {

class GlobalPipelineCache
{
public:
    static bool Init()
    {
        VkPipelineCacheCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
        if (vkCreatePipelineCache(GlobalRenderContext::GetDevice(), &createInfo, nullptr, &s_PipelineCache) != VK_SUCCESS)
        {
            return false;
        }
        return true;
    }
    static bool Release()
    {
        vkDestroyPipelineCache(GlobalRenderContext::GetDevice(), s_PipelineCache, nullptr);
        return true;
    }
    static VkPipelineCache Get()
    {
        return s_PipelineCache;
    }

private:
    static VkPipelineCache s_PipelineCache;
};

}
} // namespace Aether::vk