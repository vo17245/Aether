#include "Window.h"
#include "Core/Base.h"
#include "Render/Vulkan/Def.h"
#include "Render/Vulkan/GraphicsCommandBuffer.h"
#include "Render/Vulkan/Semaphore.h"
#include "vulkan/vulkan_core.h"
#include <cstdlib>
#include <memory>
#include <variant>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <optional>
#include <vector>
#include "Event.h"
#include "WindowContext.h"
#include "Layer.h"
#include "Render/Vulkan/FrameBuffer.h"
#include "Render/Vulkan/ImageView.h"
#include <algorithm>
#include "WindowContext.h"
#include "EventBaseMethod.h"
#include "Render/Vulkan/GlobalRenderContext.h"
#include <ranges>
namespace Aether {
using namespace vk;
Window::~Window()
{
    m_Layers.clear();
    ReleaseRenderObject();
    ReleaseSyncObjects();
    if (m_Handle != nullptr)
    {
        WindowContext::Remove(m_Handle);
        glfwDestroyWindow(m_Handle);
    }
}

Window::Window(Window&& other) noexcept
{
    m_Handle = other.m_Handle;
    other.m_Handle = nullptr;
}
Window& Window::operator=(Window&& other) noexcept
{
    if (this != &other)
    {
        if (m_Handle != nullptr)
            glfwDestroyWindow(m_Handle);
        m_Handle = other.m_Handle;
        other.m_Handle = nullptr;
    }
    return *this;
}
GLFWwindow* Window::GetHandle() const
{
    return m_Handle;
}
bool Window::ShouldClose() const
{
    return glfwWindowShouldClose(m_Handle);
}
void Window::DispatchEvent()
{
    for (auto& e : m_Event)
    {
        m_Input.OnEvent(e);
    }
    for (auto& e : m_Event)
    {
        for (auto& layer : m_Layers)
        {
            layer->OnEvent(e);
            bool isHandled = EventBaseIsHandled(e);
            if (isHandled)
            {
                break;
            }
        }
    }
    m_Event.clear();
}
/**
 * @brief Create a window
 * @return pointer to the window, nullptr if failed
 */
Window* Window::Create(int width, int height, const char* title)
{
    // create handle
    auto* handle = CreateGlfwHandle(width, height, title);
    if (handle == nullptr)
    {
        return nullptr;
    }
    // instance
    Window* window = new Window(handle);
    // register
    WindowContext::Register(handle, window);
    return window;
}
/**
 * @note
 *   只是把layer挂在到window，不转移所有权
 */
void Window::PushLayer(Layer* layer)
{
    m_Layers.emplace_back(layer);
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        m_LayerRenderFinishedSemaphore[i].emplace_back(Semaphore::CreateScope());
    }
    layer->OnAttach(this);
}
void Window::PopLayer(Layer* layer)
{
    auto iter = std::find(m_Layers.begin(), m_Layers.end(), layer);
    if (iter != m_Layers.end())
    {
        
        (*iter)->OnDetach();
        m_Layers.erase(iter);
        vkDeviceWaitIdle(vk::GRC::GetDevice());// 防止删除Semaphore时还在使用
        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
        {
            m_LayerRenderFinishedSemaphore[i].pop_back();
        }
    }
    else
    {
        assert(false && "PopLayer: layer not found");
    }
}
VkSwapchainKHR Window::GetSwapchain() const
{
    return m_SwapChain;
}
const std::vector<VkImage>& Window::GetImages() const
{
    return m_SwapChainImages;
}
std::vector<VkImage>& Window::GetImages()
{
    return m_SwapChainImages;
}
const std::vector<ImageView>& Window::GetImageViews() const
{
    return m_SwapChainImageViews;
}
std::vector<ImageView>& Window::GetImageViews()
{
    return m_SwapChainImageViews;
}
const std::vector<FrameBuffer>& Window::GetFrameBuffers() const
{
    return m_SwapChainFramebuffers;
}
std::vector<FrameBuffer>& Window::GetFrameBuffers()
{
    return m_SwapChainFramebuffers;
}
void Window::CreateCommandBuffer()
{
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        m_GraphicsCommandBuffer[i] = vk::GraphicsCommandBuffer::CreateScope(vk::GRC::GetGraphicsCommandPool());
    }
}
bool Window::CreateRenderObject()
{
    VkInstance instance = GRC::GetInstance();
    VkPhysicalDevice physicalDevice = GRC::GetPhysicalDevice();
    VkDevice device = GRC ::GetDevice();
    CreateSurface(instance);
    CreateSwapChain(instance, physicalDevice, device);
    CreateImageViews();
    CreateRenderPass(m_SwapChainImageFormat);
    CreateFramebuffers();
    CreateSyncObjects();
    CreateCommandBuffer();
    return true;
}
void Window::ReleaseRenderObject()
{
    m_SwapChainFramebuffers.clear();
    for (auto& renderPass : m_RenderPass)
    {
        renderPass.reset();
    }
    m_SwapChainImageViews.clear();
    m_SwapChainImages.clear();
    if (m_SwapChain != VK_NULL_HANDLE)
    {
        vkDestroySwapchainKHR(GRC::GetDevice(), m_SwapChain, nullptr);
        m_SwapChain = VK_NULL_HANDLE;
    }

    if (m_Surface != VK_NULL_HANDLE)
    {
        vkDestroySurfaceKHR(GRC::GetInstance(), m_Surface, nullptr);
        m_Surface = VK_NULL_HANDLE;
    }
    for (size_t i : std::views::iota(0, MAX_FRAMES_IN_FLIGHT))
    {
        m_GraphicsCommandBuffer[i].reset();
    }
}
VkSurfaceKHR Window::GetSurface() const
{
    return m_Surface;
}
RenderPass& Window::GetRenderPass(uint32_t index) const
{
    return *m_RenderPass[index];
}

/**
 * @brief create framebuffers
 */
void Window::CreateFramebuffers()
{
    for (size_t i = 0; i < m_SwapChainImageViews.size(); i++)
    {
        auto framebufferOpt = FrameBuffer::Create(*m_RenderPass[i],
                                                  m_SwapChainExtent,
                                                  m_SwapChainImageViews[i]);
        if (!framebufferOpt.has_value())
        {
            assert(false && "FrameBuffer::Create failed");
        }

        m_SwapChainFramebuffers.emplace_back(std::move(framebufferOpt.value()));
    }
}
/**
 * @brief create surface
 */
VkResult Window::CreateSurface(VkInstance instance)
{
    return glfwCreateWindowSurface(instance, m_Handle, nullptr, &m_Surface);
}
Window::Window(GLFWwindow* window) :
    m_Handle(window)
{
}
/**
 *@brief Create a glfw window handle
 */
GLFWwindow* Window::CreateGlfwHandle(int width, int height, const char* title)
{
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    return glfwCreateWindow(width, height, title, nullptr, nullptr);
}
/**
 *@brief Create swapchain ;swapchain images ; setup SwapChainImageFormat ;setup SwapChainExtent
 */
void Window::CreateSwapChain(VkInstance instance, VkPhysicalDevice physicalDevice, VkDevice device)
{
    SwapChainSupportDetails swapChainSupport = querySwapChainSupport(physicalDevice, m_Surface);

    VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
    VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
    VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities, m_Handle);

    uint32_t imageCount = 0;
    // uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
    if (swapChainSupport.capabilities.maxImageCount < MAX_FRAMES_IN_FLIGHT)
    {
        assert(false && "swapChainSupport.capabilities.maxImageCount < MAX_FRAMES_IN_FLIGHT");
    }
    else
    {
        imageCount = MAX_FRAMES_IN_FLIGHT;
    }

    if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount)
    {
        imageCount = swapChainSupport.capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = m_Surface;

    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    QueueFamilyIndices indices = findQueueFamilies(physicalDevice, m_Surface);
    uint32_t queueFamilyIndices[] = {indices.graphicsFamily.value(), indices.presentFamily.value()};

    if (indices.graphicsFamily != indices.presentFamily)
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    }
    else
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }

    createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;

    createInfo.oldSwapchain = VK_NULL_HANDLE;

    if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &m_SwapChain) != VK_SUCCESS)
    {
        assert(false&&"failed to create swap chain!");
    }

    vkGetSwapchainImagesKHR(device, m_SwapChain, &imageCount, nullptr);
    m_SwapChainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(device, m_SwapChain, &imageCount, m_SwapChainImages.data());

    m_SwapChainImageFormat = surfaceFormat.format;
    m_SwapChainExtent = extent;
}
/**
 *@brief Create swapchain image views
 */
void Window::CreateImageViews()
{
    auto size = GetSize();
    for (size_t i = 0; i < m_SwapChainImages.size(); i++)
    {
        auto imageViewOpt = ImageView::Create(m_SwapChainImages[i], m_SwapChainImageFormat, size.x(), size.y());
        if (!imageViewOpt.has_value())
        {
            assert(false && "ImageView::Create failed");
        }
        m_SwapChainImageViews.emplace_back(std::move(imageViewOpt.value()));
    }
}
Vec2i Window::GetSize() const
{
    int width, height;
    glfwGetFramebufferSize(m_Handle, &width, &height);
    return Vec2i(width, height);
}

void Window::OnUpdate(float sec)
{
    for (auto& layer : m_Layers)
    {
        layer->OnUpdate(sec);
    }
}
void Window::OnRender()
{
    if (m_Layers.empty()) return;
    if (GetSize().x() == 0 || GetSize().y() == 0) return;
    // wait last frame
    // m_InFlightFence->Wait();
    m_CommandBufferFences[m_CurrentFrame]->Wait();
    // async acquire next image
    uint32_t imageIndex;
    VkResult result = vkAcquireNextImageKHR(GRC::GetDevice(),
                                            m_SwapChain,
                                            UINT64_MAX,
                                            m_ImageAvailableSemaphore[m_CurrentFrame]->GetHandle(),
                                            VK_NULL_HANDLE,
                                            &imageIndex);
    if (result != VK_SUCCESS)
    {
        return;
    }
    m_CommandBufferFences[m_CurrentFrame]->Reset();

    // record command buffer
    m_GraphicsCommandBuffer[m_CurrentFrame]->Begin();
    size_t lastFrame = (m_CurrentFrame + MAX_FRAMES_IN_FLIGHT - 1) % MAX_FRAMES_IN_FLIGHT;
    bool anyLayerNeedRender = false;
    for (size_t i = 0; i < m_Layers.size(); ++i)
    {
        auto* layer = m_Layers[i];
        layer->OnRender(*m_RenderPass[m_CurrentFrame],
                        m_SwapChainFramebuffers[imageIndex],
                        *m_GraphicsCommandBuffer[m_CurrentFrame]);
    }
    m_GraphicsCommandBuffer[m_CurrentFrame]->End();
    // commit command buffer
    auto imageAvailableSemaphore = m_ImageAvailableSemaphore[m_CurrentFrame]->GetHandle();
    static VkPipelineStageFlags  stage=VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT;
    auto renderFinishedSemaphore = m_RenderFinishedSemaphore[m_CurrentFrame]->GetHandle();
    m_GraphicsCommandBuffer[m_CurrentFrame]->Submit(1,
    &imageAvailableSemaphore,
      &stage, 1, &renderFinishedSemaphore, m_CommandBufferFences[m_CurrentFrame]->GetHandle());
    // async present
    
    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = &renderFinishedSemaphore;
    VkSwapchainKHR swapChains[] = {m_SwapChain};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &imageIndex;
    vkQueuePresentKHR(GRC::GetPresentQueue().GetHandle(), &presentInfo);
}
bool Window::CreateSyncObjects()
{
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        m_CommandBufferFences[i] = std::make_unique<Fence>(std::move(Fence::Create(true).value()));
    }
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        m_ImageAvailableSemaphore[i] = std::make_unique<Semaphore>(std::move(Semaphore::Create().value()));
    }
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        m_RenderFinishedSemaphore[i] = std::make_unique<Semaphore>(std::move(Semaphore::Create().value()));
    }
    return true;
}
bool Window::ReleaseSyncObjects()
{
    for (auto& fence : m_CommandBufferFences)
    {
        fence.reset();
    }
    for (auto& semaphore : m_ImageAvailableSemaphore)
    {
        semaphore.reset();
    }
    for (auto& semaphore : m_RenderFinishedSemaphore)
    {
        semaphore.reset();
    }
    for (auto& arr:m_LayerRenderFinishedSemaphore)
    {
        for (auto& semaphore:arr)
        {
            semaphore.reset();
        }
    }
    return true;
}
void Window::PushEvent(const Event& e)
{
    m_Event.emplace_back(e);
}
Input& Window::GetInput()
{
    return m_Input;
}

void Window::OnLoopEnd()
{
    for (auto* layer : m_Layers)
    {
        layer->OnLoopEnd();
    }
}
void Window::OnLoopBegin()
{
    for (auto* layer : m_Layers)
    {
        layer->OnLoopBegin();
    }
}
void Window::CreateRenderPass(VkFormat format)
{
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        m_RenderPass[i] = std::make_unique<RenderPass>(*RenderPass::CreateForPresent(format));
    }
}
bool Window::ReleaseVulkanObjects()
{
    ReleaseRenderObject();
    return ReleaseSyncObjects();
}
} // namespace Aether