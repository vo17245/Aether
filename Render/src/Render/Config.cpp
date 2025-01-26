#include "Config.h"
namespace Aether
{
    namespace Render
    {
        Api Config::RenderApi=Api::Vulkan;
        int Config::VulkanApiVersion=VK_API_VERSION_1_3;
        int Config::VulkanApiVersionNumber=130;
    }
}