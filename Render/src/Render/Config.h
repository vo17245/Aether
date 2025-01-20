#pragma once
#include "vulkan/vulkan.h"
namespace Aether
{
    namespace Render
    {
        class StaticConfig
        {
        public:
            static const inline constexpr int VulkanApiVersion=VK_API_VERSION_1_3;
            static const inline constexpr int VulkanApiVersionNumber=130;
        };
    }
}