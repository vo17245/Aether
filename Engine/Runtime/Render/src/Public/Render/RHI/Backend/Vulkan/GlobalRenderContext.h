#pragma once
#include <Render/RHI/Backend/Vulkan/GraphicsCommandPool.h>
#include <Render/RHI/Backend/Vulkan/RenderContext.h>
#include "vulkan/vulkan_core.h"
#include <mutex>
#include "DynamicDescriptorPool.h"
#include <Render/Config.h>

namespace Aether
{
namespace vk
{

class GlobalRenderContext
{
public:
    static void Init(const InitResource& resource, const RenderContext::Config& config);
    static void Set(RenderContext* context);
    static RenderContext& Get();
    static VkInstance GetInstance();
    static VkPhysicalDevice GetPhysicalDevice();
    static VkDevice GetDevice();
    static Queue& GetGraphicsQueue();
    static Queue& GetPresentQueue();
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
    /**
     * @brief get current thread's DynamicDescriptorPool
     */
    static DynamicDescriptorPool& GetDynamicDescriptorPool(uint32_t frameIndex);
    static DynamicDescriptorPool& GetCurrentFrameDynamicDescriptorPool()
    {
        return GetDynamicDescriptorPool(s_FrameIndex);
    }
    static void SetFrameIndex(uint32_t frameIndex)
    {
        s_FrameIndex = frameIndex;
    }
    static uint32_t GetFrameIndex()
    {
        return s_FrameIndex;
    }
    static const QueueFamilyIndices& GetQueueFamilyIndicesRef()
    {
        return s_Context->m_QueueFamilyIndices;
    }

private:
    static RenderContext* s_Context;
    static thread_local std::unique_ptr<GraphicsCommandPool> s_GraphicsCommandPool;
    static thread_local std::once_flag s_GraphicsCommandPoolFlag;
    static thread_local std::unique_ptr<DynamicDescriptorPool>
        s_DynamicDescriptorPool[Render::Config::InFlightFrameResourceSlots];
    static thread_local std::once_flag s_DynamicDescriptorPoolFlag;
    static uint32_t s_FrameIndex;
};

using GRC = GlobalRenderContext;
} // namespace vk
} // namespace Aether