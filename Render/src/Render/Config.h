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
            static Api RenderApi;
            static int VulkanApiVersion;
            static int VulkanApiVersionNumber;
        };
    }
}