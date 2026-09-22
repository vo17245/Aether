#include "Render/RHI/Backend/Vulkan/GlobalRenderContext.h"
#include "Render/RHI/Backend/Vulkan/Allocator.h"
#include "Render/RHI/Backend/Vulkan/GraphicsCommandPool.h"
#include <stdexcept>
namespace Aether
{
namespace vk
{
namespace
{
bool g_RenderContextWasInitialized = false;
std::mutex g_RuntimeOwnerMutex;
std::thread::id g_RuntimeOwner;
}

RenderContext* GlobalRenderContext::s_Context = nullptr;
thread_local std::unique_ptr<GraphicsCommandPool> GlobalRenderContext::s_GraphicsCommandPool;
thread_local std::once_flag GlobalRenderContext::s_GraphicsCommandPoolFlag;
thread_local std::unique_ptr<DynamicDescriptorPool>
    GlobalRenderContext::s_DynamicDescriptorPool[Render::Config::InFlightFrameResourceSlots];
thread_local std::once_flag GlobalRenderContext::s_DynamicDescriptorPoolFlag;
uint32_t GlobalRenderContext::s_FrameIndex = 0;
GraphicsCommandPool& GlobalRenderContext::GetGraphicsCommandPool()
{
    AssertRuntimeRenderThread();
    std::call_once(GlobalRenderContext::s_GraphicsCommandPoolFlag,
                   []() { GlobalRenderContext::s_GraphicsCommandPool = GraphicsCommandPool::CreateScope(); });
    return *GlobalRenderContext::s_GraphicsCommandPool;
}
DynamicDescriptorPool& GlobalRenderContext::GetDynamicDescriptorPool(uint32_t frameIndex)
{
    AssertRuntimeRenderThread();
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
    AssertRuntimeRenderThread();
    return s_Context->m_Device;
}
Queue& GlobalRenderContext::GetGraphicsQueue()
{
    AssertRuntimeRenderThread();
    return s_Context->m_GraphicsQueue;
}
Queue& GlobalRenderContext::GetPresentQueue()
{
    AssertRuntimeRenderThread();
    return s_Context->m_PresentQueue;
}

QueueFamilyIndices GlobalRenderContext::GetQueueFamilyIndices()
{
    return s_Context->m_QueueFamilyIndices;
}
void GlobalRenderContext::Init(const InitResource& resource, const RenderContext::Config& config)
{
    if (s_Context != nullptr || g_RenderContextWasInitialized)
        throw std::logic_error("GlobalRenderContext does not support repeated initialization");
    g_RenderContextWasInitialized = true;
    auto* renderContext = new vk::RenderContext();
    vk::GlobalRenderContext::Set(renderContext);
    renderContext->Init(resource, config);
    // vk::GRC::GetMainWindow().CreateSyncObjects();
    vk::Allocator::Init();
}
void GlobalRenderContext::Cleanup()
{
    CleanupCurrentThreadResources();
    if (vk::Allocator::IsInitialized()) vk::Allocator::Release();
    if (s_Context)
    {
        s_Context->Cleanup();
        delete s_Context;
        s_Context = nullptr;
    }
}
void GlobalRenderContext::CleanupCurrentThreadResources()
{
    s_GraphicsCommandPool.reset();
    for (auto& pool : s_DynamicDescriptorPool) pool.reset();
}
void GlobalRenderContext::BindRuntimeRenderThread()
{
    std::lock_guard lock(g_RuntimeOwnerMutex);
    if (g_RuntimeOwner != std::thread::id{} && g_RuntimeOwner != std::this_thread::get_id())
        throw std::logic_error("Vulkan runtime owner is already bound to another thread");
    g_RuntimeOwner = std::this_thread::get_id();
}
void GlobalRenderContext::UnbindRuntimeRenderThread()
{
    std::lock_guard lock(g_RuntimeOwnerMutex);
    if (g_RuntimeOwner != std::thread::id{} && g_RuntimeOwner != std::this_thread::get_id())
        throw std::logic_error("only the Vulkan runtime owner may release ownership");
    g_RuntimeOwner = {};
}
void GlobalRenderContext::AssertRuntimeRenderThread()
{
    std::lock_guard lock(g_RuntimeOwnerMutex);
    if (g_RuntimeOwner != std::thread::id{} && g_RuntimeOwner != std::this_thread::get_id())
        throw std::logic_error("runtime Vulkan operation attempted outside RenderThread");
}
} // namespace vk
} // namespace Aether
