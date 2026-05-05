#include "Render/RHI/Backend/Vulkan/GlobalRenderContext.h"
#include "Render/RHI/Backend/Vulkan/Allocator.h"
#include "Render/RHI/Backend/Vulkan/GraphicsCommandPool.h"
namespace Aether
{
namespace vk
{

RenderContext* GlobalRenderContext::s_Context = nullptr;
thread_local std::unique_ptr<GraphicsCommandPool> GlobalRenderContext::s_GraphicsCommandPool;
thread_local std::once_flag GlobalRenderContext::s_GraphicsCommandPoolFlag;
thread_local std::unique_ptr<DynamicDescriptorPool>
    GlobalRenderContext::s_DynamicDescriptorPool[Render::Config::InFlightFrameResourceSlots];
thread_local std::once_flag GlobalRenderContext::s_DynamicDescriptorPoolFlag;
uint32_t GlobalRenderContext::s_FrameIndex = 0;
GraphicsCommandPool& GlobalRenderContext::GetGraphicsCommandPool()
{
    std::call_once(GlobalRenderContext::s_GraphicsCommandPoolFlag,
                   []() { GlobalRenderContext::s_GraphicsCommandPool = GraphicsCommandPool::CreateScope(); });
    return *GlobalRenderContext::s_GraphicsCommandPool;
}
DynamicDescriptorPool& GlobalRenderContext::GetDynamicDescriptorPool(uint32_t frameIndex)
{
    std::call_once(GlobalRenderContext::s_DynamicDescriptorPoolFlag, []() {
        for (size_t i = 0; i < Render::Config::InFlightFrameResourceSlots; ++i)
        {
            GlobalRenderContext::s_DynamicDescriptorPool[i] = std::make_unique<DynamicDescriptorPool>();
        }
    });
    return *GlobalRenderContext::s_DynamicDescriptorPool[frameIndex];
}
void GlobalRenderContext::Set(RenderContext* context)
{
    s_Context = context;
}
RenderContext& GlobalRenderContext::Get()
{
    return *s_Context;
}
VkInstance GlobalRenderContext::GetInstance()
{
    return s_Context->m_Instance;
}
VkPhysicalDevice GlobalRenderContext::GetPhysicalDevice()
{
    return s_Context->m_PhysicalDevice;
}
VkDevice GlobalRenderContext::GetDevice()
{
    return s_Context->m_Device;
}
Queue& GlobalRenderContext::GetGraphicsQueue()
{
    return s_Context->m_GraphicsQueue;
}
Queue& GlobalRenderContext::GetPresentQueue()
{
    return s_Context->m_PresentQueue;
}

QueueFamilyIndices GlobalRenderContext::GetQueueFamilyIndices()
{
    return s_Context->m_QueueFamilyIndices;
}
void GlobalRenderContext::Init(const InitResource& resource, const RenderContext::Config& config)
{
    auto* renderContext = new vk::RenderContext();
    vk::GlobalRenderContext::Set(renderContext);
    renderContext->Init(resource, config);
    // vk::GRC::GetMainWindow().CreateSyncObjects();
    vk::Allocator::Init();
}
void GlobalRenderContext::Cleanup()
{
    s_GraphicsCommandPool.reset();
    vk::Allocator::Release();
    s_Context->Cleanup();
}
} // namespace vk
} // namespace Aether