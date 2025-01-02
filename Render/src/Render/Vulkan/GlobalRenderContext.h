#pragma once
#include "RenderContext.h"
#include "vulkan/vulkan_core.h"
namespace Aether {
namespace vk {

class GlobalRenderContext
{
public:
    static void Init(Window* window,bool enableValidationLayers=false);
    static void Set(RenderContext* context);
    static RenderContext& Get();
    static VkInstance GetInstance();
    static VkPhysicalDevice GetPhysicalDevice();
    static VkDevice GetDevice();
    static Queue GetGraphicsQueue();
    static Queue GetPresentQueue();
    static VkSurfaceKHR GetMainWindowSurface();
    static Window& GetMainWindow();
    // static VkCommandPool GetGraphicsCommandPool();
    static VkSwapchainKHR GetSwapChain();
    static QueueFamilyIndices GetQueueFamilyIndices();
    static void Cleanup();

private:
    static RenderContext* s_Context;
};

using GRC = GlobalRenderContext;
}
} // namespace Aether::vk