#include "Window.h"
#include "Core/Base.h"
#include "Render/PixelFormat.h"
#include "WindowEvent.h"
#include "vulkan/vulkan_core.h"
#include <cstdlib>
#include <stdexcept>
#include <memory>
#include <variant>
#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>
#include <optional>
#include <vector>
#include "Event.h"
#include "WindowContext.h"
#include "Layer.h"
#include <algorithm>
#include "WindowContext.h"
#include "EventBaseMethod.h"
#include <ranges>
#include "ImGui/Compat/ImGuiApi.h"
#include <ImGui/Backend/imgui_impl_rendergraph.h>
#include <Debug/Log.h>
#include <Render/Threads/SubmitThread.h>

namespace Aether
{
Window::~Window()
{
    if(!m_Layers.empty())
    {
        Render::SubmitThread::WaitIdle();
        m_RenderGraph.reset();
        for(auto* layer:m_Layers) layer->OnDetach();
    }
    m_Layers.clear();
    ReleaseVulkanObjects();
    if (m_Handle != nullptr)
    {
        WindowContext::Remove(m_Handle);
        SDL_DestroyWindow(m_Handle);
    }
}

Window::Window(Window&& other) noexcept
{
    m_Handle = other.m_Handle;
    other.m_Handle = nullptr;
    m_InFlightResources = std::move(other.m_InFlightResources);
}
Window& Window::operator=(Window&& other) noexcept
{
    if (this != &other)
    {
        if (m_Handle != nullptr)
            SDL_DestroyWindow(m_Handle);
        m_Handle = other.m_Handle;
        other.m_Handle = nullptr;
        m_InFlightResources = std::move(other.m_InFlightResources);
    }
    return *this;
}
SDL_Window* Window::GetHandle() const
{
    return m_Handle;
}
bool Window::ShouldClose() const
{
    return m_ShouldClose;
}
void Window::DispatchEvent()
{
    for (auto& e : m_Event)
    {
        if (std::holds_alternative<WindowFocusLostEvent>(e) ||
            std::holds_alternative<WindowMinimizedEvent>(e))
            m_Input.ClearPressedKeys();
        m_Input.OnEvent(e);
    }
    for (auto& e : m_Event)
    {
        EventHandler.Broadcast(e);
    }
    for (auto& e : m_Event)
    {
        for (auto iter = m_Layers.rbegin(); iter != m_Layers.rend(); ++iter)
        {
            auto* layer = *iter;
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
    auto* handle = CreateSdlHandle(param);
    if (handle == nullptr)
    {
        return nullptr;
    }
    // instance
    Window* window = new Window(handle);
    // register
    WindowContext::Register(handle, window);
    window->m_ImGuiClearEnable = param.imGuiEnableClear;
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
bool Window::PopLayer(Layer* layer)
{
    auto iter = std::find(m_Layers.begin(), m_Layers.end(), layer);
    if (iter != m_Layers.end())
    {
        Render::SubmitThread::WaitIdle();
        auto* detached = *iter;
        m_Layers.erase(iter);
        CreateRenderGraph();
        detached->OnDetach();
        return true;
    }
    return false;
}
rhi::SwapChain* Window::GetSwapChain() const
{
    return m_SwapChain.get();
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
            rhi::CommandList(vk::GraphicsCommandBuffer::Create(vk::GRC::GetGraphicsCommandPool()).value());
    }
}
bool Window::CreateRenderObject()
{
    VkInstance instance = vk::GRC::GetInstance();
    VkPhysicalDevice physicalDevice = vk::GRC::GetPhysicalDevice();
    VkDevice device = vk::GRC ::GetDevice();
    if (CreateSurface(instance) != VK_SUCCESS)
    {
        return false;
    }
    CreateSwapChain(instance, physicalDevice, device);
    CreateImageViews();
    CreateSyncObjects();
    CreateCommandBuffer();
    bool res = CreateFinalImage();
    if (!res)
    {
        return false;
    }
    if (!m_ResourceArena) InitRenderGraphResource();
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
    // create final image(layer will render to final image)
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
    {
        rhi::TextureDesc desc{
            .usages=PackFlags(rhi::TextureUsage::ColorAttachment, rhi::TextureUsage::Sample, rhi::TextureUsage::TransferSrc),
            .pixelFormat=PixelFormat::RGBA8888,
            .width=(uint32_t)size.x(),
            .height=(uint32_t)size.y(),
            .layout=rhi::TextureLayout::ShaderReadOnly
        };
        auto textureOpt = rhi::Texture2D::Create(desc);
        if (!textureOpt)
        {
            assert(false && "DeviceTexture::CreateForTexture failed");
            return false;
        }
        auto& texture = textureOpt;
        m_FinalTextures[i] = std::move(texture);
    }
    return true;
}
void Window::ReleaseFinalImage()
{
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
        m_FinalImageViews[i] = rhi::TextureView();
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
    {
        m_FinalTextures[i] = rhi::Texture2D();
    }
}
void Window::ReleaseRenderObject()
{
    ImGuiWindowContextDestroy();
    m_SwapChainImageViews.clear();
    m_SwapChainImages.clear();
    m_SwapChain.reset();

    for (size_t i : std::views::iota(0, MAX_FRAMES_IN_FLIGHT))
    {
        m_GraphicsCommandBuffer[i] = rhi::CommandList();
    }
    // RenderGraph-owned views must be destroyed before their external final images.
    m_RenderGraph.reset();
    ReleaseFinalImage();
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
    if (m_Surface != VK_NULL_HANDLE)
    {
        return VK_SUCCESS;
    }
    if (instance == VK_NULL_HANDLE || m_Handle == nullptr)
    {
        return VK_ERROR_INITIALIZATION_FAILED;
    }
    return SDL_Vulkan_CreateSurface(m_Handle, instance, nullptr, &m_Surface)
               ? VK_SUCCESS
               : VK_ERROR_INITIALIZATION_FAILED;
}
Window::Window(SDL_Window* window) : m_Handle(window), m_InFlightResources(CreateScope<InFlightResourceAllocator>(MAX_FRAMES_IN_FLIGHT))
{
}
/**
 *@brief Create an SDL window handle
 */
SDL_Window* Window::CreateSdlHandle(const WindowCreateParam& param)
{
    SDL_WindowFlags flags = SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY;
    if (param.noDecorate)
        flags |= SDL_WINDOW_BORDERLESS;
    return SDL_CreateWindow(param.title.c_str(), param.width, param.height, flags);
}
void Window::SetSize(uint32_t width, uint32_t height)
{
    if (m_Handle == nullptr)
    {
        assert(false && "Window handle is null");
        return;
    }
    SDL_SetWindowSize(m_Handle, static_cast<int>(width), static_cast<int>(height));
}
/**
 *@brief Create swapchain ;swapchain images ; setup SwapChainImageFormat ;setup SwapChainExtent
 */
void Window::CreateSwapChain(VkInstance instance, VkPhysicalDevice physicalDevice, VkDevice device)
{
    vk::SwapChainSupportDetails swapChainSupport = vk::querySwapChainSupport(physicalDevice, m_Surface);

    VkSurfaceFormatKHR surfaceFormat = vk::chooseSwapSurfaceFormat(swapChainSupport.formats);
    VkPresentModeKHR presentMode = vk::chooseSwapPresentMode(swapChainSupport.presentModes);
    m_PresentMode = presentMode;
    LogI("[vulkan] choose swapchain present mode: {}", (int)presentMode);
    const auto requestedSize = GetSize();
    VkExtent2D extent = vk::chooseSwapExtent(
        swapChainSupport.capabilities,
        {static_cast<uint32_t>(requestedSize.x()), static_cast<uint32_t>(requestedSize.y())});

    // MAX_FRAMES_IN_FLIGHT controls synchronization, while the surface
    // capabilities control the valid swapchain image count. In particular,
    // maxImageCount == 0 means that there is no maximum.
    uint32_t imageCount = std::max(
        swapChainSupport.capabilities.minImageCount,
        static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT));

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
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;

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

    {
        VkSwapchainKHR swapChain;
        if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapChain) != VK_SUCCESS)
        {
            assert(false && "failed to create swap chain!");
        }
        m_SwapChain = CreateScope<rhi::SwapChain>(vk::SwapChain(std::move(swapChain)));
    }

    vkGetSwapchainImagesKHR(device, m_SwapChain->GetVk().GetHandle(), &imageCount, nullptr);
    m_SwapChainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(device, m_SwapChain->GetVk().GetHandle(), &imageCount, m_SwapChainImages.data());

    m_SwapChainImageFormat = surfaceFormat.format;
    m_SwapChainColorSpace = surfaceFormat.colorSpace;
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
    int width = 0;
    int height = 0;
    SDL_GetWindowSizeInPixels(m_Handle, &width, &height);
    return Vec2i(width, height);
}
Vec2i Window::GetPosition() const
{
    int x = 0;
    int y = 0;
    SDL_GetWindowPosition(m_Handle, &x, &y);
    return Vec2i(x, y);
}
Vec2f Window::GetCursorPosition() const
{
    float x = 0.0f;
    float y = 0.0f;
    SDL_GetMouseState(&x, &y);
    return Vec2f(x, y);
}
bool Window::IsMouseButtonPressed(MouseButtonCode button) const
{
    Uint8 sdlButton = SDL_BUTTON_LEFT;
    if (button == MouseButtonCode::Right)
        sdlButton = SDL_BUTTON_RIGHT;
    else if (button == MouseButtonCode::Middle)
        sdlButton = SDL_BUTTON_MIDDLE;
    return (SDL_GetMouseState(nullptr, nullptr) & SDL_BUTTON_MASK(sdlButton)) != 0;
}
void Window::SetSize(int width, int height)
{
    SDL_SetWindowSize(m_Handle, width, height);
}
void Window::SetPosition(int width, int height)
{
    SDL_SetWindowPosition(m_Handle, width, height);
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
    // Keep SDL text events enabled for the engine's CharacterInputEvent stream.
    SDL_StartTextInput(m_Handle);
    for (auto& layer : m_Layers)
    {
        layer->OnImGuiUpdate();
    }
    ImGui::Render();
}

void Window::OnRender()
{
    if (m_Minilized || GetSize().x() == 0 || GetSize().y() == 0)
    {
        return;
    }
   
    // wait for render resource
    m_CommandBufferFences[m_CurrentFrame]->GetVkFence().Wait();
    // acquire next image
    {
        auto imageAcquireSubmit = CreateScope<Render::VkImageAcquireSubmit>();
        imageAcquireSubmit->swapChain = &m_SwapChain->GetVk();
        imageAcquireSubmit->signalSemaphore = &m_ImageAvailableSemaphore[m_CurrentFrame]->GetVkSemaphore();
        imageAcquireSubmit->timeoutNs = std::numeric_limits<uint64_t>::max();
        imageAcquireSubmit->result = &m_ImageAcquireResult;
        imageAcquireSubmit->semaphore = &m_ImageAcquireSemaphore;
        Render::SubmitThread::PushSubmit(std::move(imageAcquireSubmit));
    }
    m_ImageAcquireSemaphore.acquire();
    if (m_ImageAcquireResult.status == Render::ImageAcquireStatus::OutOfDate)
    {
        Render::SubmitThread::WaitIdle();
        ReleaseRenderObject();
        if (!CreateRenderObject())
            throw std::runtime_error("Failed to recreate out-of-date swapchain");
        CreateRenderGraph();
        return;
    }
    if (m_ImageAcquireResult.status == Render::ImageAcquireStatus::NotReady ||
        m_ImageAcquireResult.status == Render::ImageAcquireStatus::Timeout)
        return;
    if (m_ImageAcquireResult.status != Render::ImageAcquireStatus::Success)
        throw std::runtime_error("Failed to acquire swapchain image");
    OnImageAcquired(m_ImageAcquireResult);
}
bool Window::CreateSyncObjects()
{
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        m_CommandBufferFences[i] = std::make_unique<rhi::Fence>(std::move(vk::Fence::Create(true).value()));
    }
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        m_ImageAvailableSemaphore[i] = std::make_unique<rhi::Fence>(std::move(vk::Semaphore::Create().value()));
    }
    m_RenderFinishedSemaphores.clear();
    m_RenderFinishedSemaphores.reserve(m_SwapChainImages.size());
    for (size_t i = 0; i < m_SwapChainImages.size(); ++i)
    {
        auto semaphore = vk::Semaphore::Create();
        if (!semaphore)
        {
            return false;
        }
        m_RenderFinishedSemaphores.push_back(
            std::make_unique<rhi::Fence>(std::move(semaphore.value())));
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
    m_RenderFinishedSemaphores.clear();

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
    m_ResourcePool.reset();
    m_ResourceArena.reset();
    for (size_t i=0; i<MAX_FRAMES_IN_FLIGHT; ++i)
    {
        m_FinalTextureArenaIds[i]={};m_FinalViewArenaIds[i]={};
    }
    const bool syncObjectsReleased = ReleaseSyncObjects();
    if (m_Surface != VK_NULL_HANDLE)
    {
        SDL_Vulkan_DestroySurface(vk::GRC::GetInstance(), m_Surface, nullptr);
        m_Surface = VK_NULL_HANDLE;
    }
    return syncObjectsReleased;
}
rhi::Texture2D& Window::GetFinalTexture(uint32_t index)
{
    return m_FinalTextures[index];
}
bool Window::ResizeFinalImage(const Vec2u& size)
{
    VkExtent2D extent{(uint32_t)size.x(), (uint32_t)size.y()};
    // create final image(layer will render to final image)
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
        m_FinalImageViews[i] = rhi::TextureView();
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
    {
        rhi::TextureDesc desc{
            .usages=PackFlags(rhi::TextureUsage::ColorAttachment, rhi::TextureUsage::Sample, rhi::TextureUsage::TransferSrc),
            .pixelFormat=PixelFormat::RGBA8888,
            .width=(uint32_t)size.x(),
            .height=(uint32_t)size.y(),
            .layout=rhi::TextureLayout::ShaderReadOnly
        };
        auto textureOpt = rhi::Texture2D::Create(desc);
        if (!textureOpt)
        {
            assert(false && "rhi::Texture2D::CreateForColorAttachment failed");
            return false;
        }
        auto& texture = textureOpt;
        m_FinalTextures[i] = std::move(texture);
        m_FinalImageViews[i] = m_FinalTextures[i].CreateImageView(rhi::TextureViewDesc{});
    }
    return true;
}

void Window::OnWindowResize(const Vec2u& size)
{
    if (size.x() == 0 || size.y() == 0)
    {
        m_Minilized = true;
        return; // no need to resize
    }
    m_Minilized = false;
    Render::SubmitThread::WaitIdle();
    assert(ResizeFinalImage(size) && "failed to resize window final image");
    CreateRenderGraph();
}
void Window::InitRenderGraphResource()
{
    m_ResourceArena = CreateScope<RenderGraph::ResourceArena>();
    m_ResourcePool = CreateScope<RenderGraph::ResourceLruPool>(m_ResourceArena.get());
}
void Window::CreateRenderGraph()
{
    LogD("Rebuild RenderGraph");
    // The old graph has finished recording. Its external registrations are
    // independent of the Window-owned images and can now be recycled.
    m_RenderGraph.reset();
    for (size_t i=0; i<MAX_FRAMES_IN_FLIGHT; ++i)
    {
        if (m_FinalViewArenaIds[i].handle.IsValid()) m_ResourceArena->Destroy(m_FinalViewArenaIds[i]);
        if (m_FinalTextureArenaIds[i].handle.IsValid()) m_ResourceArena->Destroy(m_FinalTextureArenaIds[i]);
        m_FinalViewArenaIds[i]={};m_FinalTextureArenaIds[i]={};
    }
    // create
    m_RenderGraph = CreateScope<RenderGraph::RenderGraph>(m_ResourceArena.get(), m_ResourcePool.get());
    // import final image
    RenderGraph::ResourceId<rhi::Texture2D> finalImageResourceIds[MAX_FRAMES_IN_FLIGHT];
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
    {
        finalImageResourceIds[i] = m_ResourceArena->Import(&m_FinalTextures[i]);
        m_FinalTextureArenaIds[i] = finalImageResourceIds[i];
    }
    RenderGraph::TextureDesc finalImageDesc;
    finalImageDesc.usages = PackFlags(rhi::TextureUsage::ColorAttachment, rhi::TextureUsage::Sample, rhi::TextureUsage::TransferSrc);
    finalImageDesc.pixelFormat = PixelFormat::RGBA8888;
    finalImageDesc.width = m_FinalTextures[0].GetWidth();
    finalImageDesc.height = m_FinalTextures[0].GetHeight();
    finalImageDesc.layout = rhi::TextureLayout::ShaderReadOnly;
    m_FinalImageAccessId = m_RenderGraph->Import(
        "FinalImage", finalImageDesc,
        std::span<const RenderGraph::ResourceId<rhi::Texture2D>>(finalImageResourceIds, MAX_FRAMES_IN_FLIGHT));
    // import final image view
    RenderGraph::ResourceId<rhi::TextureView> finalImageViewResourceIds[MAX_FRAMES_IN_FLIGHT];
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
    {
        finalImageViewResourceIds[i] = m_ResourceArena->Import(&m_FinalImageViews[i]);
        m_FinalViewArenaIds[i] = finalImageViewResourceIds[i];
    }
    RenderGraph::TextureViewDesc finalImageViewDesc;
    finalImageViewDesc.texture = m_FinalImageAccessId;
    finalImageViewDesc.desc = rhi::TextureViewDesc();
    m_RenderGraph->Import(
        "FinalImageView", finalImageViewDesc,
        std::span<const RenderGraph::ResourceId<rhi::TextureView>>(finalImageViewResourceIds, MAX_FRAMES_IN_FLIGHT));

    // call each layer's RegisterRenderPasses function
    for (auto* layer : m_Layers)
    {
        layer->OnBuildRenderGraph(*m_RenderGraph);
    }
    // compile
    m_RenderGraph->Compile();
}
void Window::ImGuiRecordCommandBuffer(rhi::CommandList& commandBuffer)
{
    auto frame = std::make_unique<ImGuiApi::WindowContext::Frame>();
    auto& graph = frame->graph;
    auto& texture = m_FinalTextures[m_CurrentFrame];
    auto resource = frame->arena.Import(&texture);
    RenderGraph::TextureDesc desc{};
    desc.width = texture.GetWidth();
    desc.height = texture.GetHeight();
    desc.pixelFormat = texture.GetFormat();
    desc.usages = texture.GetUsages();
    desc.layout = rhi::TextureLayout::ShaderReadOnly;
    auto target = graph.Import<rhi::Texture2D>("ImGui.FinalImage", desc,
        std::span<const RenderGraph::ResourceId<rhi::Texture2D>>(&resource, 1));
    // With no scene layers, initialize the image even if UI clearing is disabled.
    const bool clear = m_ImGuiClearEnable || m_Layers.empty();
    Vec4f color = m_ImGuiClearColor;
    color.x() *= color.w();
    color.y() *= color.w();
    color.z() *= color.w();
    ImGui_ImplRenderGraph_RenderDrawData(ImGui::GetDrawData(), graph, target, clear, color);
    graph.Compile();
    graph.SetCommandBuffer(&commandBuffer);
    graph.Execute();
    m_ImGuiContext.frames[m_CurrentFrame] = std::move(frame);
}
void Window::ImGuiWindowContextDestroy()
{
    for (auto& frame : m_ImGuiContext.frames)
        frame.reset();
}
void Window::Maximize()
{
    SDL_MaximizeWindow(m_Handle);
}
void Window::OnUpload()
{
    for (auto* layer : m_Layers)
    {
        layer->OnUpload(m_PendingUploadList);
    }
}
void Window::SetCursorPosition(double x, double y)
{
    SDL_WarpMouseInWindow(m_Handle, static_cast<float>(x), static_cast<float>(y));
}
void Window::SetCursorMode(CursorMode mode)
{
    (void)TrySetCursorMode(mode);
}
bool Window::TrySetCursorMode(CursorMode mode)
{
    if (m_Handle == nullptr) return false;
    if (mode == CursorMode::Disabled)
    {
        if (!SDL_SetWindowRelativeMouseMode(m_Handle, true)) return false;
        if (SDL_HideCursor()) return true;
        SDL_SetWindowRelativeMouseMode(m_Handle, false);
        SDL_ShowCursor();
        return false;
    }
    if (mode == CursorMode::Normal)
    {
        if (!SDL_SetWindowRelativeMouseMode(m_Handle, false)) return false;
        if (SDL_ShowCursor()) return true;
        SDL_SetWindowRelativeMouseMode(m_Handle, true);
        SDL_HideCursor();
        return false;
    }
    if (mode == CursorMode::Hidden)
    {
        if (!SDL_SetWindowRelativeMouseMode(m_Handle, false)) return false;
        return SDL_HideCursor();
    }
    return false;
}
void Window::OnImageAcquired(const Render::ImageAcquireResult& result)
{
    if (result.status != Render::ImageAcquireStatus::Success)
    {
        assert(false && "unknown error");
        return;
    }
    uint32_t imageIndex = result.imageIndex;
    auto& imageAvailableSemaphore = *m_ImageAvailableSemaphore[m_CurrentFrame];

    // The allocator changes slots only after the fence wait and successful image
    // acquisition. Layer callbacks can therefore write this slot without racing
    // the previous submission. Keep the fence signaled until all callbacks have
    // completed so an exception cannot leave an unsignaled, unsubmitted fence.
    m_InFlightResources->SetCurrentFrame(m_CurrentFrame);

    for (auto* layer : m_Layers)
    {
        layer->OnFrameBegin();
    }

    // Reset only when a submission will signal this fence. This deliberately
    // happens after OnFrameBegin(), whose resource writes may throw.
    m_CommandBufferFences[m_CurrentFrame]->GetVkFence().Reset();
    m_ResourcePool->OnFrameBegin();

    // Only age uploads once a render slot is available. CPU updates while a
    // window is minimized must not retire staging data that has not been submitted.
    m_ImGuiContext.frames[m_CurrentFrame].reset();
    m_PendingUploadList.OnUpdate(false);
    ImGui_ImplRenderGraph_UpdateTextures(ImGui::GetDrawData(), m_PendingUploadList);

    // record command buffer
    auto& curCommandBufferVk = m_GraphicsCommandBuffer[m_CurrentFrame].GetVk();
    auto& curCommandBuffer = m_GraphicsCommandBuffer[m_CurrentFrame];
    curCommandBufferVk.Reset();
    curCommandBufferVk.Begin();
    // curCommandBuffer.BeginRenderPass(curRenderPass, curFrameBuffer,clearColor);
    // record transfer command here
    m_PendingUploadList.RecordCommand(curCommandBuffer);

    // Record the RenderGraph. It owns the final-image layout transitions.
    if (m_RenderGraph)
    {
        m_RenderGraph->SetCommandBuffer(&m_GraphicsCommandBuffer[m_CurrentFrame]);
        m_RenderGraph->SetCurrentFrame(m_CurrentFrame);
        m_RenderGraph->Execute();
    }

    // Composite UI into the final image using a RenderGraph task before presentation.
    ImGuiRecordCommandBuffer(curCommandBuffer);

    // Composite the RenderGraph final image into the acquired swapchain image.
    auto& finalImage = m_FinalTextures[m_CurrentFrame].GetVk();
    VkImageMemoryBarrier beforeBlit[2]{};
    beforeBlit[0].sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    beforeBlit[0].srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
    beforeBlit[0].dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
    beforeBlit[0].oldLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    beforeBlit[0].newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    beforeBlit[0].srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    beforeBlit[0].dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    beforeBlit[0].image = finalImage.GetHandle();
    beforeBlit[0].subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
    beforeBlit[1].sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    beforeBlit[1].srcAccessMask = 0;
    beforeBlit[1].dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    // The acquired image is completely overwritten, including on its first use.
    beforeBlit[1].oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    beforeBlit[1].newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    beforeBlit[1].srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    beforeBlit[1].dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    beforeBlit[1].image = m_SwapChainImages[imageIndex];
    beforeBlit[1].subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
    vkCmdPipelineBarrier(curCommandBufferVk.GetHandle(), VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                         VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 2, beforeBlit);

    const auto finalSize = GetSize();
    VkImageBlit blit{};
    blit.srcSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    blit.srcOffsets[1] = {finalSize.x(), finalSize.y(), 1};
    blit.dstSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    blit.dstOffsets[1] = {static_cast<int32_t>(m_SwapChainExtent.width),
                          static_cast<int32_t>(m_SwapChainExtent.height), 1};
    vkCmdBlitImage(curCommandBufferVk.GetHandle(), finalImage.GetHandle(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                   m_SwapChainImages[imageIndex], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &blit,
                   VK_FILTER_LINEAR);

    VkImageMemoryBarrier afterBlit[2] = {beforeBlit[0], beforeBlit[1]};
    afterBlit[0].srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
    afterBlit[0].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    afterBlit[0].oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    afterBlit[0].newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    afterBlit[1].srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    afterBlit[1].dstAccessMask = 0;
    afterBlit[1].oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    afterBlit[1].newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    vkCmdPipelineBarrier(curCommandBufferVk.GetHandle(), VK_PIPELINE_STAGE_TRANSFER_BIT,
                         VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
                         0, 0, nullptr, 0, nullptr, 2, afterBlit);

    curCommandBufferVk.End();
    // A presentation semaphore can only be reused after its swapchain image is acquired again.
    assert(imageIndex < m_RenderFinishedSemaphores.size());
    auto& renderFinishedSemaphore = *m_RenderFinishedSemaphores[imageIndex];
    {
        auto submit = CreateScope<Render::VkCommandSubmit>();
        submit->commandBuffer = &m_GraphicsCommandBuffer[m_CurrentFrame].GetVk();
        submit->signalFence = &m_CommandBufferFences[m_CurrentFrame]->GetVkFence();
        submit->waitSemaphores.push_back(&imageAvailableSemaphore.GetVkSemaphore());
        submit->waitStages.push_back(Render::PipelineSyncStage::AllCommands);
        submit->signalSemaphores.push_back(&renderFinishedSemaphore.GetVkSemaphore());
        submit->queue = &vk::GRC::GetGraphicsQueue();
        Render::SubmitThread::PushSubmit(std::move(submit));
    }
    // m_GraphicsCommandBuffer[m_CurrentFrame].GetVk().Submit(1, &imageAvailableSemaphoreHandle, &stage, 1,
    //                                                        &renderFinishedSemaphore,
    //                                                        m_CommandBufferFences[m_CurrentFrame]->GetVk().GetHandle());
    //  async present

    // VkPresentInfoKHR presentInfo{};
    // presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    // presentInfo.waitSemaphoreCount = 1;
    // presentInfo.pWaitSemaphores = &renderFinishedSemaphore;
    // VkSwapchainKHR swapChains[] = {m_SwapChain->GetVk().GetHandle()};
    // presentInfo.swapchainCount = 1;
    // presentInfo.pSwapchains = swapChains;
    // presentInfo.pImageIndices = &imageIndex;
    // vkQueuePresentKHR(vk::GRC::GetPresentQueue().GetHandle(), &presentInfo);
    {
        auto present = CreateScope<Render::VkPresentSubmit>();
        present->imageIndex = imageIndex;
        present->swapChain = &m_SwapChain->GetVk();
        present->waitSemaphores.push_back(&renderFinishedSemaphore.GetVkSemaphore());
        Render::SubmitThread::PushSubmit(std::move(present));
    }

    // forward current frame index
    m_CurrentFrame = (m_CurrentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}
} // namespace Aether
