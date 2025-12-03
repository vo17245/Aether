#include "GlobalRenderContext.h"
#include "Allocator.h"
#include "GraphicsCommandPool.h"
#include "Window/Window.h"
namespace Aether {
namespace vk {

RenderContext* GlobalRenderContext::s_Context=nullptr;
thread_local std::unique_ptr<GraphicsCommandPool> GlobalRenderContext::s_GraphicsCommandPool;
thread_local std::once_flag GlobalRenderContext::s_GraphicsCommandPoolFlag;
GraphicsCommandPool& GlobalRenderContext::GetGraphicsCommandPool()
{
    std::call_once(GlobalRenderContext::s_GraphicsCommandPoolFlag, []() {
        GlobalRenderContext::s_GraphicsCommandPool = GraphicsCommandPool::CreateScope();
    });
    return *GlobalRenderContext::s_GraphicsCommandPool;
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
VkSurfaceKHR GlobalRenderContext::GetMainWindowSurface()
{
    return s_Context->m_Window->GetSurface();
}
Window& GlobalRenderContext::GetMainWindow()
{
    return *s_Context->m_Window;
}
// VkCommandPool GlobalRenderContext::GetGraphicsCommandPool()
//{
//     return s_Context->m_GraphicsCommandPool;
// }
VkSwapchainKHR GlobalRenderContext::GetSwapChain()
{
    auto* deviceSwapChain=s_Context->m_Window->GetSwapChain();
    if(!deviceSwapChain)
    {
        return VK_NULL_HANDLE;
    }
    return deviceSwapChain->GetVk().GetHandle();
}
QueueFamilyIndices GlobalRenderContext::GetQueueFamilyIndices()
{
    return s_Context->m_QueueFamilyIndices;
}
void GlobalRenderContext::Init(Window* window, const RenderContext::Config& config)
{
    auto* renderContext = new vk::RenderContext();
    vk::GlobalRenderContext::Set(renderContext);
    renderContext->Init(window,config);
    //vk::GRC::GetMainWindow().CreateSyncObjects();
    vk::Allocator::Init();
    window->CreateFinalImage();
    window->InitRenderGraphResource();
}
void GlobalRenderContext::Cleanup()
{
    s_GraphicsCommandPool.reset();
    vk::Allocator::Release();
    s_Context->Cleanup();
}
}
} // namespace Aether::vk