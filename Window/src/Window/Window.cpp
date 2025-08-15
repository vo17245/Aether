#include "Window.h"
#include "Core/Base.h"
#include "GammaFilter.h"
#include "Render/PixelFormat.h"
#include "Render/RenderApi/DeviceDescriptorPool.h"
#include "Render/RenderApi/DeviceTexture.h"
#include "Render/Vulkan/Def.h"
#include "Render/Vulkan/DescriptorPool.h"
#include "Render/Vulkan/GraphicsCommandBuffer.h"
#include "Render/Vulkan/RenderPass.h"
#include "Render/Vulkan/Semaphore.h"
#include "Render/Vulkan/Texture2D.h"
#include "WindowEvent.h"
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
namespace Aether
{
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

    layer->OnAttach(this);
}
void Window::PopLayer(Layer* layer)
{
    auto iter = std::find(m_Layers.begin(), m_Layers.end(), layer);
    if (iter != m_Layers.end())
    {
        (*iter)->OnDetach();
        m_Layers.erase(iter);
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
const std::vector<vk::ImageView>& Window::GetImageViews() const
{
    return m_SwapChainImageViews;
}
std::vector<vk::ImageView>& Window::GetImageViews()
{
    return m_SwapChainImageViews;
}
const std::vector<vk::FrameBuffer>& Window::GetFrameBuffers() const
{
    return m_SwapChainFramebuffers;
}
std::vector<vk::FrameBuffer>& Window::GetFrameBuffers()
{
    return m_SwapChainFramebuffers;
}
void Window::CreateCommandBuffer()
{
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        m_GraphicsCommandBuffer[i] =
            DeviceCommandBuffer(vk::GraphicsCommandBuffer::Create(vk::GRC::GetGraphicsCommandPool()).value());
    }
}
bool Window::CreateRenderObject()
{
    VkInstance instance = vk::GRC::GetInstance();
    VkPhysicalDevice physicalDevice = vk::GRC::GetPhysicalDevice();
    VkDevice device = vk::GRC ::GetDevice();
    CreateSurface(instance);
    CreateSwapChain(instance, physicalDevice, device);
    CreateImageViews();
    CreateRenderPass(m_SwapChainImageFormat);
    CreateFramebuffers();
    CreateSyncObjects();
    CreateCommandBuffer();
    bool res = CreateFinalImage();
    if (!res)
    {
        return false;
    }
    return true;
}
bool Window::CreateFinalImage()
{
    auto size = GetSize();
    auto textureOpt = DeviceTexture::CreateForColorAttachment(size.x(), size.y(), PixelFormat::RGBA8888);
    if (!textureOpt)
    {
        assert(false && "DeviceTexture::CreateForTexture failed");
        return false;
    }
    auto& texture = *textureOpt;
    auto renderPassOpt = vk::RenderPass::CreateDefault(PixelFormat::RGBA8888);
    if (!renderPassOpt)
    {
        assert(false && "RenderPass::CreateDefault failed");
        return false;
    }
    m_FinalRenderPass = DeviceRenderPass(std::move(renderPassOpt.value()));

    VkExtent2D extent{(uint32_t)size.x(), (uint32_t)size.y()};
    auto framebufferOpt =
        vk::FrameBuffer::Create(m_FinalRenderPass.GetVk(), extent, texture.GetOrCreateDefaultImageView().GetVk());
    if (!framebufferOpt)
    {
        assert(false && "FrameBuffer::Create failed");
        return false;
    }
    auto& framebuffer = *framebufferOpt;
    auto poolOpt = DeviceDescriptorPool::Create();
    if (!poolOpt)
    {
        assert(false && "DeviceDescriptorPool::Create failed");
        return false;
    }
    auto& pool = *poolOpt;
    auto filterOpt = WindowInternal::GammaFilter::Create(*m_RenderPass[0], pool);
    if (!filterOpt)
    {
        assert(false && "GammaFilter::Create failed");
        return false;
    }
    m_GammaFilter = CreateScope<WindowInternal::GammaFilter>(std::move(filterOpt.value()));
    m_GammaFilter->SetGamma(2.2f);
    texture.SyncTransitionLayout(DeviceImageLayout::Undefined, DeviceImageLayout::Texture);

    m_FinalTexture = std::move(texture);
    m_FinalFrameBuffer = DeviceFrameBuffer(std::move(framebuffer));
    m_DescriptorPool = std::move(pool);
}
void Window::ReleaseFinalImage()
{
    m_FinalTexture = DeviceTexture();
    m_FinalFrameBuffer = DeviceFrameBuffer();
    m_FinalRenderPass = DeviceRenderPass();
    m_GammaFilter.reset();
    m_DescriptorPool = DeviceDescriptorPool();
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
        vkDestroySwapchainKHR(vk::GRC::GetDevice(), m_SwapChain, nullptr);
        m_SwapChain = VK_NULL_HANDLE;
    }

    if (m_Surface != VK_NULL_HANDLE)
    {
        vkDestroySurfaceKHR(vk::GRC::GetInstance(), m_Surface, nullptr);
        m_Surface = VK_NULL_HANDLE;
    }
    for (size_t i : std::views::iota(0, MAX_FRAMES_IN_FLIGHT))
    {
        m_GraphicsCommandBuffer[i] = DeviceCommandBuffer();
    }
    ReleaseFinalImage();
}
VkSurfaceKHR Window::GetSurface() const
{
    return m_Surface;
}
vk::RenderPass& Window::GetRenderPass(uint32_t index) const
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
        auto framebufferOpt = vk::FrameBuffer::Create(*m_RenderPass[i], m_SwapChainExtent, m_SwapChainImageViews[i]);
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
Window::Window(GLFWwindow* window) : m_Handle(window)
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
void Window::SetSize(uint32_t width, uint32_t height)
{
    if (m_Handle == nullptr)
    {
        assert(false && "Window handle is null");
        return;
    }
    glfwSetWindowSize(m_Handle, width, height);
    OnWindowResize(Vec2u(width, height));
}
/**
 *@brief Create swapchain ;swapchain images ; setup SwapChainImageFormat ;setup SwapChainExtent
 */
void Window::CreateSwapChain(VkInstance instance, VkPhysicalDevice physicalDevice, VkDevice device)
{
    vk::SwapChainSupportDetails swapChainSupport = vk::querySwapChainSupport(physicalDevice, m_Surface);

    VkSurfaceFormatKHR surfaceFormat = vk::chooseSwapSurfaceFormat(swapChainSupport.formats);
    VkPresentModeKHR presentMode = vk::chooseSwapPresentMode(swapChainSupport.presentModes);
    VkExtent2D extent = vk::chooseSwapExtent(swapChainSupport.capabilities, m_Handle);

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

    vk::QueueFamilyIndices indices = vk::findQueueFamilies(physicalDevice, m_Surface);
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
        assert(false && "failed to create swap chain!");
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
        auto imageViewOpt = vk::ImageView::Create(m_SwapChainImages[i], m_SwapChainImageFormat, size.x(), size.y());
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
void Window::WaitLastFrameComplete()
{
    m_CommandBufferFences[m_CurrentFrame]->Wait();
}
void Window::OnRender()
{
    for (auto* layer : m_Layers)
    {
        layer->OnFrameBegin();
    }
    m_DescriptorPool.Clear();
    if (m_Layers.empty())
        return;
    if (GetSize().x() == 0 || GetSize().y() == 0)
        return;

    // async acquire next image
    uint32_t imageIndex;
    VkResult result =
        vkAcquireNextImageKHR(vk::GRC::GetDevice(), m_SwapChain, UINT64_MAX,
                              m_ImageAvailableSemaphore[m_CurrentFrame]->GetHandle(), VK_NULL_HANDLE, &imageIndex);
    if (result != VK_SUCCESS)
    {
        return;
    }
    m_CommandBufferFences[m_CurrentFrame]->Reset();

    // record command buffer
    size_t lastFrame = (m_CurrentFrame + MAX_FRAMES_IN_FLIGHT - 1) % MAX_FRAMES_IN_FLIGHT;
    auto& curCommandBuffer = m_GraphicsCommandBuffer[m_CurrentFrame].GetVk();
    auto& curRenderPass = *m_RenderPass[m_CurrentFrame];
    auto& curFrameBuffer = m_SwapChainFramebuffers[imageIndex];
    Vec4f clearColor(0.0f, 0.0f, 0.0f, 1.0f);
    curCommandBuffer.Reset();
    curCommandBuffer.Begin();
    // curCommandBuffer.BeginRenderPass(curRenderPass, curFrameBuffer,clearColor);
    m_FinalTexture.AsyncTransitionLayout(DeviceImageLayout::Texture, DeviceImageLayout::ColorAttachment,
                                         curCommandBuffer);
    for (size_t i = 0; i < m_Layers.size(); ++i)
    {
        auto* layer = m_Layers[i];
        // layer->OnRender(*m_RenderPass[m_CurrentFrame],
        //                 m_SwapChainFramebuffers[imageIndex],
        //                 *m_GraphicsCommandBuffer[m_CurrentFrame]);
        layer->OnRender(m_GraphicsCommandBuffer[m_CurrentFrame]);
    }
    // render to screen
    m_FinalTexture.AsyncTransitionLayout(DeviceImageLayout::ColorAttachment, DeviceImageLayout::Texture,
                                         curCommandBuffer);
    curCommandBuffer.BeginRenderPass(*m_RenderPass[m_CurrentFrame], m_SwapChainFramebuffers[imageIndex],
                                     Vec4f(0.0, 0.0, 0.0, 1.0));
    curCommandBuffer.SetScissor(0, 0, GetSize().x(), GetSize().y());
    curCommandBuffer.SetViewport(0, 0, GetSize().x(), GetSize().y());
    m_GammaFilter->Render(m_FinalTexture, m_SwapChainFramebuffers[imageIndex], curCommandBuffer, m_DescriptorPool);
    curCommandBuffer.EndRenderPass();
    // curCommandBuffer.EndRenderPass();
    curCommandBuffer.End();
    // commit command buffer
    auto imageAvailableSemaphore = m_ImageAvailableSemaphore[m_CurrentFrame]->GetHandle();
    static VkPipelineStageFlags stage = VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT;
    auto renderFinishedSemaphore = m_RenderFinishedSemaphore[m_CurrentFrame]->GetHandle();
    m_GraphicsCommandBuffer[m_CurrentFrame].GetVk().Submit(1, &imageAvailableSemaphore, &stage, 1,
                                                           &renderFinishedSemaphore,
                                                           m_CommandBufferFences[m_CurrentFrame]->GetHandle());
    // async present

    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = &renderFinishedSemaphore;
    VkSwapchainKHR swapChains[] = {m_SwapChain};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &imageIndex;
    vkQueuePresentKHR(vk::GRC::GetPresentQueue().GetHandle(), &presentInfo);
}
bool Window::CreateSyncObjects()
{
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        m_CommandBufferFences[i] = std::make_unique<vk::Fence>(std::move(vk::Fence::Create(true).value()));
    }
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        m_ImageAvailableSemaphore[i] = std::make_unique<vk::Semaphore>(std::move(vk::Semaphore::Create().value()));
    }
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        m_RenderFinishedSemaphore[i] = std::make_unique<vk::Semaphore>(std::move(vk::Semaphore::Create().value()));
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

void Window::CreateRenderPass(VkFormat format)
{
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        m_RenderPass[i] = std::make_unique<vk::RenderPass>(*vk::RenderPass::CreateForPresent(format));
    }
}
bool Window::ReleaseVulkanObjects()
{
    ReleaseRenderObject();
    return ReleaseSyncObjects();
}
DeviceTexture& Window::GetFinalTexture()
{
    return m_FinalTexture;
}
bool Window::ResizeFinalImage(const Vec2u& size)
{
    auto textureOpt = DeviceTexture::CreateForColorAttachment(size.x(), size.y(), PixelFormat::RGBA8888);
    if (!textureOpt)
    {
        assert(false && "DeviceTexture::CreateForTexture failed");
        return false;
    }
    auto& texture = *textureOpt;
    VkExtent2D extent{(uint32_t)size.x(), (uint32_t)size.y()};
    auto framebufferOpt =
        vk::FrameBuffer::Create(m_FinalRenderPass.GetVk(), extent, texture.GetOrCreateDefaultImageView().GetVk());
    if (!framebufferOpt)
    {
        assert(false && "FrameBuffer::Create failed");
        return false;
    }
    auto& framebuffer = *framebufferOpt;
    texture.SyncTransitionLayout(DeviceImageLayout::Undefined, DeviceImageLayout::Texture);
    m_FinalTexture = std::move(texture);
    m_FinalFrameBuffer = DeviceFrameBuffer(std::move(framebuffer));
    return true;
}
void Window::OnWindowResize(const Vec2u& size)
{
    if (size.x() == 0 || size.y() == 0)
    {
        return; // no need to resize
    }
    assert(ResizeFinalImage(size) && "failed to resize window final image");
}
} // namespace Aether