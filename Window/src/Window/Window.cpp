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
#include "ImGui/Compat/ImGuiApi.h"
#include <Render/Vulkan/Allocator.h>
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
Window* Window::Create(const WindowCreateParam& param)
{
    // create handle
    auto* handle = CreateGlfwHandle(param);
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
    CreateRenderGraph();
}
void Window::PushLayers(const std::span<Layer*>& layers)
{
    for (auto* layer : layers)
    {
        PushLayer(layer);
    }
    CreateRenderGraph();
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
    CreateSyncObjects();
    CreateCommandBuffer();
    bool res = CreateFinalImage();
    if (!res)
    {
        return false;
    }
    InitRenderGraphResource();
    return true;
}
bool Window::CreateFinalImage()
{
    auto size = GetSize();
    VkExtent2D extent{(uint32_t)size.x(), (uint32_t)size.y()};
    // create tonemap render pass

    auto renderPassOpt = vk::RenderPass::CreateForPresent(m_SwapChainImageFormat);
    if (!renderPassOpt)
    {
        assert(false && "RenderPass::CreateDefault failed");
        return false;
    }
    m_TonemapRenderPass = DeviceRenderPass(std::move(renderPassOpt.value()));
    // create final image(layer will render to final image)
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
    {
        auto textureOpt = DeviceTexture::CreateForColorAttachment(size.x(), size.y(), PixelFormat::RGBA8888);
        if (!textureOpt)
        {
            assert(false && "DeviceTexture::CreateForTexture failed");
            return false;
        }
        auto& texture = *textureOpt;
        texture.SyncTransitionLayout(DeviceImageLayout::Undefined, DeviceImageLayout::Texture);

        m_FinalTextures[i] = std::move(texture);
    }

    // create tonemap framebuffer
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
    {
        auto framebufferOpt = vk::FrameBuffer::Create(m_TonemapRenderPass.GetVk(), extent, m_SwapChainImageViews[i]);
        if (!framebufferOpt)
        {
            assert(false && "FrameBuffer::Create failed");
            return false;
        }
        auto& framebuffer = *framebufferOpt;
        m_TonemapFrameBuffers[i] = std::move(framebuffer);
    }
    // create descriptor pool
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
    {
        auto poolOpt = DeviceDescriptorPool::Create();
        if (!poolOpt)
        {
            assert(false && "DeviceDescriptorPool::Create failed");
            return false;
        }
        auto& pool = *poolOpt;
        m_DescriptorPools[i] = std::move(pool);
    }
    // create gamma filter

    auto filterOpt = WindowInternal::GammaFilter::Create(m_TonemapRenderPass.GetVk(), m_DescriptorPools[0]);
    if (!filterOpt)
    {
        assert(false && "GammaFilter::Create failed");
        return false;
    }
    m_GammaFilter = CreateScope<WindowInternal::GammaFilter>(std::move(filterOpt.value()));
    // create image view
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
    {
        DeviceImageViewDesc desc;
        m_FinalImageViews[i] = m_FinalTextures[i].CreateImageView(desc);
    }
    return true;
}
void Window::ReleaseFinalImage()
{
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
    {
        m_FinalTextures[i] = DeviceTexture();
        m_TonemapFrameBuffers[i] = DeviceFrameBuffer();
        m_DescriptorPools[i] = DeviceDescriptorPool();
        m_FinalImageViews[i] = DeviceImageView();
    }

    m_TonemapRenderPass = DeviceRenderPass();
    m_GammaFilter.reset();
}
void Window::ReleaseRenderObject()
{
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
    m_RenderGraph.reset();
    m_ResourcePool.reset();
    m_ResourceArena.reset();
}
VkSurfaceKHR Window::GetSurface() const
{
    return m_Surface;
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
GLFWwindow* Window::CreateGlfwHandle(const WindowCreateParam& param)
{
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    if (param.noDecorate)
    {
        // 设置窗口无边框
        glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
    }

    return glfwCreateWindow(param.width, param.height, param.title.c_str(), nullptr, nullptr);
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
void Window::SetSize(int width, int height)
{
    glfwSetWindowSize(m_Handle, width, height);
    OnWindowResize(Vec2u(width, height));
}
void Window::SetPosition(int width, int height)
{
    glfwSetWindowPos(m_Handle, width, height);
}
void Window::OnUpdate(float sec)
{
    for (auto& layer : m_Layers)
    {
        layer->OnUpdate(sec);
    }
    bool anyLayerNeedRebuild = false;
    for (auto& layer : m_Layers)
    {
        if (layer->NeedRebuildRenderGraph())
        {
            anyLayerNeedRebuild = true;
            break;
        }
    }
    if (anyLayerNeedRebuild)
    {
        CreateRenderGraph();
    }
    // imgui
    // Start the Dear ImGui frame
    ImGuiApi::NewFrame();
    for (auto& layer : m_Layers)
    {
        layer->OnImGuiUpdate();
    }
    ImGui::Render();
    // PendingUploadList update
    m_PendingUploadList.OnUpdate();
}

void Window::OnRender()
{
    if (GetSize().x() == 0 || GetSize().y() == 0)
    {
        return;
    }
    // if there is  buffer(not in-flight) upload, wait for all frames to complete
    if(m_PendingUploadList.NeedSync())
    {
        for(size_t i=0;i<MAX_FRAMES_IN_FLIGHT;++i)
        {
            m_CommandBufferFences[i]->Wait();
        }
    }
    // wait for render resource
    // ImGuiWaitFrameResource();
    m_CommandBufferFences[m_CurrentFrame]->Wait();
    m_CommandBufferFences[m_CurrentFrame]->Reset();
    // acquire next image
    uint32_t imageIndex;
    auto& imageAvailableSemaphore = *m_ImageAvailableSemaphore[m_CurrentFrame];
    VkResult result = vkAcquireNextImageKHR(vk::GRC::GetDevice(), m_SwapChain, UINT64_MAX,
                                            imageAvailableSemaphore.GetHandle(), VK_NULL_HANDLE, &imageIndex);

    if (result != VK_SUCCESS)
    {
        assert(false && "unknown error");
        return;
    }
    m_DescriptorPools[m_CurrentFrame].Clear();

    for (auto* layer : m_Layers)
    {
        layer->OnFrameBegin();
    }

    // record command buffer
    auto& curCommandBufferVk = m_GraphicsCommandBuffer[m_CurrentFrame].GetVk();
    auto& curCommandBuffer = m_GraphicsCommandBuffer[m_CurrentFrame];
    curCommandBufferVk.Reset();
    curCommandBufferVk.Begin();
    // curCommandBuffer.BeginRenderPass(curRenderPass, curFrameBuffer,clearColor);
    // record transfer command here
        m_PendingUploadList.RecordCommand(curCommandBuffer);

    
    // record main render command
    curCommandBuffer.ImageLayoutTransition(m_FinalTextures[m_CurrentFrame], DeviceImageLayout::Texture,
                                           DeviceImageLayout::ColorAttachment);
    m_RenderGraph->SetCommandBuffer(&m_GraphicsCommandBuffer[m_CurrentFrame]);
    m_RenderGraph->SetCurrentFrame(m_CurrentFrame);
    m_RenderGraph->Execute();
    // render to screen (tonemap)
    curCommandBuffer.ImageLayoutTransition(m_FinalTextures[m_CurrentFrame], DeviceImageLayout::ColorAttachment,
                                           DeviceImageLayout::Texture);
    curCommandBufferVk.BeginRenderPass(m_TonemapRenderPass.GetVk(), m_TonemapFrameBuffers[imageIndex].GetVk(),
                                       Vec4f(0.0, 0.0, 0.0, 1.0));
    curCommandBuffer.SetScissor(0, 0, GetSize().x(), GetSize().y());
    curCommandBuffer.SetViewport(0, 0, GetSize().x(), GetSize().y());
    m_GammaFilter->Render(m_FinalTextures[m_CurrentFrame], curCommandBuffer, m_DescriptorPools[m_CurrentFrame]);
    curCommandBufferVk.EndRenderPass();
    m_ImGuiContext.window.FrameIndex = imageIndex;
    // record imgui command
    ImGuiRecordCommandBuffer(curCommandBuffer);
    curCommandBufferVk.End();
    // commit command buffer
    auto imageAvailableSemaphoreHandle = imageAvailableSemaphore.GetHandle();
    static VkPipelineStageFlags stage = VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT;
    auto renderFinishedSemaphore = m_RenderFinishedSemaphore[m_CurrentFrame]->GetHandle();
    m_GraphicsCommandBuffer[m_CurrentFrame].GetVk().Submit(1, &imageAvailableSemaphoreHandle, &stage, 1,
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

    // forward current frame index
    m_CurrentFrame = (m_CurrentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
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

bool Window::ReleaseVulkanObjects()
{
    ReleaseRenderObject();
    return ReleaseSyncObjects();
}
DeviceTexture& Window::GetFinalTexture(uint32_t index)
{
    return m_FinalTextures[index];
}
bool Window::ResizeFinalImage(const Vec2u& size)
{
    VkExtent2D extent{(uint32_t)size.x(), (uint32_t)size.y()};
    // create final image(layer will render to final image)
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
    {
        auto textureOpt = DeviceTexture::CreateForColorAttachment(size.x(), size.y(), PixelFormat::RGBA8888);
        if (!textureOpt)
        {
            assert(false && "DeviceTexture::CreateForTexture failed");
            return false;
        }
        auto& texture = *textureOpt;
        texture.SyncTransitionLayout(DeviceImageLayout::Undefined, DeviceImageLayout::Texture);

        m_FinalTextures[i] = std::move(texture);
        m_FinalImageViews[i] = m_FinalTextures[i].CreateImageView(DeviceImageViewDesc());
    }

    // create tonemap framebuffer
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
    {
        auto framebufferOpt = vk::FrameBuffer::Create(m_TonemapRenderPass.GetVk(), extent, m_SwapChainImageViews[i]);
        if (!framebufferOpt)
        {
            assert(false && "FrameBuffer::Create failed");
            return false;
        }
        auto& framebuffer = *framebufferOpt;
        m_TonemapFrameBuffers[i] = std::move(framebuffer);
    }
    return true;
}

void Window::OnWindowResize(const Vec2u& size)
{
    if (size.x() == 0 || size.y() == 0)
    {
        return; // no need to resize
    }
    assert(ResizeFinalImage(size) && "failed to resize window final image");
    CreateRenderGraph();
    m_ImGuiContext.window.Width = size.x();
    m_ImGuiContext.window.Height = size.y();
}
void Window::InitRenderGraphResource()
{
    m_ResourceArena = CreateScope<RenderGraph::ResourceArena>();
    m_ResourcePool = CreateScope<RenderGraph::ResourceLruPool>(m_ResourceArena.get());
}
void Window::CreateRenderGraph()
{
    // create
    m_RenderGraph = CreateScope<RenderGraph::RenderGraph>(m_ResourceArena.get(), m_ResourcePool.get());
    // import final image
    RenderGraph::ResourceId<DeviceTexture> finalImageResourceIds[MAX_FRAMES_IN_FLIGHT];
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
    {
        finalImageResourceIds[i] = m_ResourceArena->Import(&m_FinalTextures[i]);
    }
    RenderGraph::TextureDesc finalImageDesc;
    finalImageDesc.usages = PackFlags(DeviceImageUsage::ColorAttachment, DeviceImageUsage::Sample);
    finalImageDesc.pixelFormat = PixelFormat::RGBA8888;
    finalImageDesc.width = m_FinalTextures[0].GetWidth();
    finalImageDesc.height = m_FinalTextures[0].GetHeight();
    finalImageDesc.layout = DeviceImageLayout::ColorAttachment;
    m_FinalImageAccessId = m_RenderGraph->Import(
        "FinalImage", finalImageDesc,
        std::span<const RenderGraph::ResourceId<DeviceTexture>>(finalImageResourceIds, MAX_FRAMES_IN_FLIGHT));
    // import final image view
    RenderGraph::ResourceId<DeviceImageView> finalImageViewResourceIds[MAX_FRAMES_IN_FLIGHT];
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
    {
        finalImageViewResourceIds[i] = m_ResourceArena->Import(&m_FinalImageViews[i]);
    }
    RenderGraph::ImageViewDesc finalImageViewDesc;
    finalImageViewDesc.texture = m_FinalImageAccessId;
    finalImageViewDesc.desc = DeviceImageViewDesc();
    m_RenderGraph->Import(
        "FinalImageView", finalImageViewDesc,
        std::span<const RenderGraph::ResourceId<DeviceImageView>>(finalImageViewResourceIds, MAX_FRAMES_IN_FLIGHT));

    // call each layer's RegisterRenderPasses function
    for (auto* layer : m_Layers)
    {
        layer->RegisterRenderPasses(*m_RenderGraph);
    }
    // compile
    m_RenderGraph->Compile();
}
static void check_vk_result(VkResult err)
{
    if (err == VK_SUCCESS)
        return;
    fprintf(stderr, "[vulkan] Error: VkResult = %d\n", err);
    if (err < 0)
        abort();
}
void Window::ImGuiWaitFrameResource()
{
    ImDrawData* draw_data = ImGui::GetDrawData();
    auto* wd = &m_ImGuiContext.window;
    ImGui_ImplVulkanH_Frame* fd = &wd->Frames[wd->FrameIndex];
    {
        auto err = vkWaitForFences(vk::GRC::GetDevice(), 1, &fd->Fence, VK_TRUE,
                                   UINT64_MAX); // wait indefinitely instead of periodically checking
        check_vk_result(err);

        err = vkResetFences(vk::GRC::GetDevice(), 1, &fd->Fence);
        check_vk_result(err);
    }
}

void Window::ImGuiRecordCommandBuffer(DeviceCommandBuffer& commandBuffer)
{
    // Rendering
    ImDrawData* draw_data = ImGui::GetDrawData();
    const bool is_minimized = (draw_data->DisplaySize.x <= 0.0f || draw_data->DisplaySize.y <= 0.0f);
    auto* wd = &m_ImGuiContext.window;
    Vec4f clear_color = m_ImGuiClearColor;
    if (!is_minimized)
    {
        wd->ClearValue.color.float32[0] = clear_color.x() * clear_color.w();
        wd->ClearValue.color.float32[1] = clear_color.y() * clear_color.w();
        wd->ClearValue.color.float32[2] = clear_color.z() * clear_color.w();
        wd->ClearValue.color.float32[3] = clear_color.w();
        ImGuiFrameRender(commandBuffer);
    }
}
void Window::ImGuiFrameRender(DeviceCommandBuffer& commandBuffer)
{
    auto* wd = &m_ImGuiContext.window;
    ImDrawData* draw_data = ImGui::GetDrawData();
    ImGui_ImplVulkanH_Frame* fd = &wd->Frames[wd->FrameIndex];
    VkSemaphore image_acquired_semaphore = wd->FrameSemaphores[wd->SemaphoreIndex].ImageAcquiredSemaphore;
    VkSemaphore render_complete_semaphore = wd->FrameSemaphores[wd->SemaphoreIndex].RenderCompleteSemaphore;
    auto& vkCommandBuffer = commandBuffer.GetVk();
    {
        VkRenderPassBeginInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        info.renderPass = wd->RenderPass;
        info.framebuffer = fd->Framebuffer;
        info.renderArea.extent.width = wd->Width;
        info.renderArea.extent.height = wd->Height;
        info.clearValueCount = 1;
        info.pClearValues = &wd->ClearValue;
        vkCmdBeginRenderPass(vkCommandBuffer.GetHandle(), &info, VK_SUBPASS_CONTENTS_INLINE);
    }

    // Record dear imgui primitives into command buffer
    ImGui_ImplVulkan_RenderDrawData(draw_data, vkCommandBuffer.GetHandle());

    // Submit command buffer
    vkCmdEndRenderPass(vkCommandBuffer.GetHandle());
}
void Window::ImGuiWindowContextInit()
{
    auto* wd = &m_ImGuiContext.window;
    wd->SurfaceFormat.format = m_SwapChainImageFormat;
    wd->Surface = m_Surface;
    wd->ImageCount = m_SwapChainImages.size();
    Vec2i size = GetSize();
    wd->Width = size.x();
    wd->Height = size.y();

    wd->SemaphoreCount = wd->ImageCount + 1;
    wd->Frames.resize(wd->ImageCount);
    wd->FrameSemaphores.resize(wd->SemaphoreCount);
    memset(wd->Frames.Data, 0, wd->Frames.size_in_bytes());
    memset(wd->FrameSemaphores.Data, 0, wd->FrameSemaphores.size_in_bytes());
    for (uint32_t i = 0; i < wd->ImageCount; i++)
    {
        wd->Frames[i].Backbuffer = m_SwapChainImages[i];
    }

    // Select Surface Format
    // const VkFormat requestSurfaceImageFormat[] = { VK_FORMAT_B8G8R8A8_UNORM, VK_FORMAT_R8G8B8A8_UNORM,
    // VK_FORMAT_B8G8R8_UNORM, VK_FORMAT_R8G8B8_UNORM }; const VkColorSpaceKHR requestSurfaceColorSpace =
    // VK_COLORSPACE_SRGB_NONLINEAR_KHR; wd->SurfaceFormat =
    // ImGui_ImplVulkanH_SelectSurfaceFormat(vk::GRC::GetPhysicalDevice(), wd->Surface, requestSurfaceImageFormat,
    // (size_t)IM_ARRAYSIZE(requestSurfaceImageFormat), requestSurfaceColorSpace);

    // Select Present Mode
    // #ifdef APP_USE_UNLIMITED_FRAME_RATE
    // VkPresentModeKHR present_modes[] = { VK_PRESENT_MODE_MAILBOX_KHR, VK_PRESENT_MODE_IMMEDIATE_KHR,
    // VK_PRESENT_MODE_FIFO_KHR };
    // #else
    //     VkPresentModeKHR present_modes[] = { VK_PRESENT_MODE_FIFO_KHR };
    // #endif
    wd->Swapchain = m_SwapChain;
    // wd->PresentMode = ImGui_ImplVulkanH_SelectPresentMode(vk::GRC::GetPhysicalDevice(), wd->Surface,
    // &present_modes[0], IM_ARRAYSIZE(present_modes)); printf("[vulkan] Selected PresentMode = %d\n", wd->PresentMode);

    // Create SwapChain, RenderPass, Framebuffer, etc.
    IM_ASSERT(m_SwapChainImages.size() >= 2);
    ImGui_ImplVulkanH_CreateOrResizeWindow(vk::GRC::GetInstance(), vk::GRC::GetPhysicalDevice(), vk::GRC::GetDevice(),
                                           wd, vk::GRC::GetQueueFamilyIndices().graphicsFamily.value(), nullptr,
                                           size.x(), size.y(), m_SwapChainImages.size());
}
void Window::ImGuiWindowContextDestroy()
{
    ImGui_ImplVulkanH_DestroyWindow(vk::GRC::GetInstance(), vk::GRC::GetDevice(), &m_ImGuiContext.window, nullptr);
}
void Window::Maximize()
{
    glfwMaximizeWindow(m_Handle);
}
void Window::OnUpload()
{
    for(auto* layer:m_Layers)
    {
        layer->OnUpload(m_PendingUploadList);
    }
}
} // namespace Aether
