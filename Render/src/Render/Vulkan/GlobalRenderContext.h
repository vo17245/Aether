#pragma once
#include "GraphicsCommandPool.h"
#include "RenderContext.h"
#include "vulkan/vulkan_core.h"
#include <mutex>

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
    /**
     * @brief get current thread's GraphicsCommandPool
    */
    static GraphicsCommandPool& GetGraphicsCommandPool();
private:
    static RenderContext* s_Context;
    static thread_local std::unique_ptr<GraphicsCommandPool> s_GraphicsCommandPool;
    static thread_local std::once_flag s_GraphicsCommandPoolFlag;
};

using GRC = GlobalRenderContext;
}
} // namespace Aether::vk