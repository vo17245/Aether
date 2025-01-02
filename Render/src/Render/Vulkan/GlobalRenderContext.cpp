#include "GlobalRenderContext.h"
#include "Window/Window.h"
namespace Aether {
namespace vk {

RenderContext* GlobalRenderContext::s_Context;
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
Queue GlobalRenderContext::GetGraphicsQueue()
{
    return s_Context->m_GraphicsQueue;
}
Queue GlobalRenderContext::GetPresentQueue()
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
    return s_Context->m_Window->GetSwapchain();
}
QueueFamilyIndices GlobalRenderContext::GetQueueFamilyIndices()
{
    return s_Context->m_QueueFamilyIndices;
}
void GlobalRenderContext::Init(Window* window, bool enableValidationLayers)
{
    auto* renderContext = new vk::RenderContext();
    renderContext->enableValidationLayers = enableValidationLayers;
    vk::GlobalRenderContext::Set(renderContext);
    renderContext->Init(window);
    vk::GRC::GetMainWindow().CreateSyncObjects();
}
void GlobalRenderContext::Cleanup()
{
    s_Context->Cleanup();
}
}
} // namespace Aether::vk