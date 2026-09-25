#pragma once
#include "Render/Render.h"
#include "CloseRequestGate.h"

#include <memory>
#include <deque>
#include <atomic>
#include <SDL3/SDL_video.h>
#include <vector>
#include "Event.h"
#include "Layer.h"
#include "Input.h"
#include "WindowState.h"
#include "Render/RenderGraph/RenderGraph.h"
#include "ImGui/Compat/WindowContext.h"
#include "ImGui/Compat/ImGuiRenderPacket.h"
#include <Render/RHI.h>
#include <Render/InFlight/InFlightResourceAllocator.h>
#include <Render/Threads/RenderThread.h>
#include <Render/Feature/RenderFeature.h>
#include <unordered_map>
#include <optional>
#include <functional>

struct ImGui_ImplRenderGraph_Backend;


namespace Aether
{
namespace vk
{
class RenderContext;

}
struct WindowCreateParam
{
    int width = 800;
    int height = 600;
    std::string title = "Default Title";
    bool noDecorate = false;
    bool imGuiEnableClear = true;
    // Diagnostic switch: preserve P7's submit-and-wait behavior for pixel and
    // ordering comparisons. Normal operation uses the bounded asynchronous queue.
    bool serialRenderThread = false;
    // Drop a frame when the bounded RenderThread queue is full; the next tick re-extracts it.
    bool nonBlockingRenderSubmit = false;
};
enum class WindowRenderIdleReason : std::uint8_t
{
    None, ResourceMaintenance, Shutdown, LayerAttach, LayerDetach, Resize, FeatureGraphRebuild, SwapchainOutOfDate
};
struct WindowRenderDiagnostics
{
    std::uint64_t renderGraphBuilds = 0;
    std::uint64_t deviceIdleWaits = 0;
    std::uint64_t nonBlockingFrameDrops = 0;
    WindowRenderIdleReason lastDeviceIdleReason = WindowRenderIdleReason::None;
};
enum class CursorMode
{
    Normal = 0,
    Hidden = 1,
    Disabled = 2,
};
class Window
{
    friend class WindowContext;
    friend class vk::RenderContext;

public:
    constexpr const static inline std::uint32_t MAX_FRAMES_IN_FLIGHT = Render::Config::MaxFramesInFlight;

public:
    ~Window();
    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;
    Window(Window&& other) noexcept;
    Window& operator=(Window&& other) noexcept;
    SDL_Window* GetHandle() const;
    bool ShouldClose() const;
    void SetCloseRequestHandler(std::function<void()> handler);
    void ConfirmClose();
    void CancelCloseRequest() noexcept;
    void DispatchEvent();
    /**
     * @brief Create a window
     * @return pointer to the window, nullptr if failed
     */
    static Window* Create(const WindowCreateParam& param);
    /**
     * @note
     *   只是把layer挂在到window，不转移所有权
     */
    void PushLayer(Layer* layer);
    void PushLayers(const std::span<Layer*>& layers);
    bool PopLayer(Layer* layer);
    void DetachAllLayers();
    bool HasLayers() const { return !m_Layers.empty(); }
    rhi::SwapChain* GetSwapChain() const;
    const std::vector<VkImage>& GetImages() const;
    std::vector<VkImage>& GetImages();
    const std::vector<vk::ImageView>& GetImageViews() const;
    std::vector<vk::ImageView>& GetImageViews();
    bool CreateRenderObject();
    void ReleaseRenderObject();
    bool CreateSyncObjects();
    bool ReleaseSyncObjects();
    bool ReleaseVulkanObjects();
    /**
     * @brief Create the Vulkan surface after the Vulkan instance is available.
     *        Calling this more than once is safe.
     */
    VkResult CreateSurface(VkInstance instance);
    void CleanupSurface();
    VkSurfaceKHR GetSurface() const;
    Vec2i GetSize() const;
    Vec2i GetPosition() const;
    Vec2f GetCursorPosition() const;
    bool IsMouseButtonPressed(MouseButtonCode button) const;
    void SetSize(int width, int height);
    void SetPosition(int width, int height);
    void OnUpdate(float sec);
    void OnRender();
    void OnUpload();
    void PushEvent(const Event& e);
    Input& GetInput();
    void Maximize();

    void ReleaseFinalImage();
    bool CreateFinalImage();
    rhi::Texture2D& GetFinalTexture(uint32_t index);
    void SetSize(uint32_t width, uint32_t height);
    uint32_t GetCurrentFrameIndex()
    {
        return m_CurrentFrame;
    }
    RenderGraph::AccessId<rhi::Texture2D> GetFinalImageAccessId() const
    {
        return m_FinalImageAccessId;
    }
    // create resource arena and lru pool
    void InitRenderGraphResource();
    inline RenderGraph::ResourceArena& GetResourceArena()
    {
        return *m_ResourceArena;
    }
    InFlightResourceAllocator& GetInFlightResourceAllocator()
    {
        return *m_InFlightResources;
    }
    inline RenderGraph::ResourceLruPool& GetResourcePool()
    {
        return *m_ResourcePool;
    }
    inline RenderGraph::RenderGraph& GetRenderGraph()
    {
        return *m_RenderGraph;
    }
    void ImGuiWindowContextDestroy();
    // Registers a window-owned RenderGraph surface as an ImGui texture handle.
    // Call on the UI thread after the render feature publishes the token.
    ImTextureID RegisterDisplaySurfaceForUi(DisplaySurfaceToken token);
    bool UnregisterDisplaySurfaceForUi(ImTextureID textureId) noexcept;
    ImGuiApi::WindowContext& GetImGuiContext()
    {
        return m_ImGuiContext;
    }
    size_t SwapChainImageCount() const
    {
        return m_SwapChainImages.size();
    }
    VkSurfaceFormatKHR GetSwapChainSurfaceFormat() const
    {
        return {m_SwapChainImageFormat, m_SwapChainColorSpace};
    }
    void SetCursorPosition(double x, double y);
    void SetCursorMode(CursorMode mode);
    bool TrySetCursorMode(CursorMode mode);
    bool IsMinilized() const
    {
        return m_WindowState.minimized;
    }
    WindowId GetId() const { return m_Id; }
    WindowRenderDiagnostics GetRenderDiagnostics() const noexcept
    {
        return {m_RenderGraphBuilds.load(std::memory_order_relaxed),
                m_DeviceIdleWaits.load(std::memory_order_relaxed),
                m_NonBlockingFrameDrops.load(std::memory_order_relaxed),
                m_LastDeviceIdleReason.load(std::memory_order_relaxed)};
    }
    const WindowState& GetWindowState() const { return m_WindowState; }
    PixelExtent GetPixelExtent() const { return m_WindowState.pixelExtent; }
    void InitializeRendering(Render::RenderThread& renderThread);
    void ShutdownRendering();
    // RenderThread onStop hook. This is idempotent and must run on its worker,
    // including after a command failure has made normal submission impossible.
    void CleanupRenderingOnRenderThread();

private:
    std::vector<Event> m_Event;
    std::vector<Layer*> m_Layers;
    std::unordered_map<Layer*, std::vector<std::shared_ptr<Render::RenderFeature>>> m_LayerRenderFeatures;
    std::vector<std::shared_ptr<Render::RenderFeature>> m_RenderFeatures;
    Scope<rhi::SwapChain> m_SwapChain;
    std::vector<VkImage> m_SwapChainImages;
    std::vector<vk::ImageView> m_SwapChainImageViews;
    VkFormat m_SwapChainImageFormat{};
    VkColorSpaceKHR m_SwapChainColorSpace{};
    VkExtent2D m_SwapChainExtent{};
    RenderGraph::ResourceId<rhi::Texture2D> m_FinalTextureArenaIds[MAX_FRAMES_IN_FLIGHT]{};
    RenderGraph::ResourceId<rhi::TextureView> m_FinalViewArenaIds[MAX_FRAMES_IN_FLIGHT]{};
    VkSurfaceKHR m_Surface = VK_NULL_HANDLE;
    SDL_Window* m_Handle = nullptr;
    std::unique_ptr<rhi::Fence> m_ImageAvailableSemaphore[MAX_FRAMES_IN_FLIGHT];
    // Presentation wait semaphores are tied to swapchain images, not frames in flight.
    std::vector<std::unique_ptr<rhi::Fence>> m_RenderFinishedSemaphores;
    Scope<rhi::Fence> m_CommandBufferFences[MAX_FRAMES_IN_FLIGHT];
    rhi::CommandList m_GraphicsCommandBuffer[MAX_FRAMES_IN_FLIGHT];
    //=========== final image
    rhi::Texture2D m_FinalTextures[MAX_FRAMES_IN_FLIGHT];
    rhi::TextureView m_FinalImageViews[MAX_FRAMES_IN_FLIGHT];
    //================================
    uint32_t m_CurrentFrame = 0;

private:
    Window(SDL_Window* window);
    /**
     *@brief Create an SDL window handle
     */
    static SDL_Window* CreateSdlHandle(const WindowCreateParam& param);
    /**
     *@brief Create swapchain ;swapchain images ; setup SwapChainImageFormat ;setup SwapChainExtent
     */
    void CreateSwapChain(VkInstance instance, VkPhysicalDevice physicalDevice, VkDevice device);
    /**
     *@brief Create swapchain image views
     */
    void CreateImageViews();
    void CreateCommandBuffer();
    bool ResizeFinalImage(const Vec2u& size);
    void OnWindowResize(const Vec2u& size);
    void RequestClose();

private:
    Input m_Input;

private: // render graph
    Scope<RenderGraph::ResourceArena> m_ResourceArena;
    Scope<RenderGraph::ResourceLruPool> m_ResourcePool;
    Scope<RenderGraph::RenderGraph> m_RenderGraph;
    Scope<InFlightResourceAllocator> m_InFlightResources;
    Scope<Render::DisplaySurfaceService> m_DisplaySurfaces;
    std::optional<Render::RenderFeatureServices> m_RenderFeatureServices;
    Render::RenderFeatureFrame m_ExtractedRenderFrame;
    RenderCommandExtraction m_ExtractedRenderCommands;
    Render::CpuFrameId m_NextCpuFrameId = 1;

    // create render graph, register final image
    // and call each layer RegisterRenderPasses function
    void CreateRenderGraph();
    void WaitForDeviceIdle(WindowRenderIdleReason reason);
    void AttachRenderFeatures(Layer& layer);
    void DetachRenderFeatures(Layer& layer);
    RenderGraph::AccessId<rhi::Texture2D> m_FinalImageAccessId;
    std::atomic_uint64_t m_RenderGraphBuilds{0};
    std::atomic_uint64_t m_DeviceIdleWaits{0};
    std::atomic_uint64_t m_NonBlockingFrameDrops{0};
    std::atomic<WindowRenderIdleReason> m_LastDeviceIdleReason{WindowRenderIdleReason::None};

private: // imgui
    bool m_ImGuiClearEnable = false;
    ImGuiApi::WindowContext m_ImGuiContext;
    ImGuiCompat::ImGuiRenderPacketExtractor m_ImGuiPacketExtractor;
    std::optional<ImGuiCompat::ImGuiRenderPacketExtractor::Extraction> m_PendingImGuiExtraction;
    std::shared_ptr<const ImGuiCompat::ImGuiRenderPacket> m_ImGuiRenderPacket;
    std::shared_ptr<const ImGuiCompat::ImGuiRenderPacket> m_LastSubmittedImGuiPacket;
    ::ImGui_ImplRenderGraph_Backend* m_ImGuiBackend = nullptr;
    Vec4f m_ImGuiClearColor = Vec4f(0.5, 0.7, 1.0, 1.0);
    void ImGuiRecordCommandBuffer(rhi::CommandList& commandBuffer);

private: // upload
    PendingUploadList m_PendingUploadList;

public:
    Delegate<void(Event&)> EventHandler;

private:
    WindowId m_Id = 0;
    WindowState m_WindowState;
    std::atomic<std::uint64_t> m_LatestWindowStateVersion = 0;
    WindowState m_RenderWindowState;
    std::uint64_t m_SubmittedWindowStateVersion = 0;
    std::uint64_t m_RenderWindowStateVersion = 0;
    CloseRequestGate m_CloseRequestGate;
    bool m_SerialRenderThread = false;
    bool m_NonBlockingRenderSubmit = false;
    std::uint64_t m_FrameSubmissionGenerations[MAX_FRAMES_IN_FLIGHT]{};
    bool m_RenderSwapchainInvalid = false;
    VkPresentModeKHR m_PresentMode;
private:
    void OnImageAcquired(std::uint32_t imageIndex, Render::RenderFrameContext& context, bool& featureFrameSubmitted);
    void UpdateWindowState(PixelExtent extent, bool minimized);
    void OnRenderThread(Render::RenderFrameContext& context,
                        Render::RenderFeatureFrame featureFrame,
                        std::shared_ptr<const ImGuiCompat::ImGuiRenderPacket> imguiPacket,
                        std::uint64_t windowStateVersion);
    void SubmitReliableAndWait(std::unique_ptr<Render::IRenderCommand> command);
    void CheckCompletedFrames(bool requireAll = false);
    void AssertRenderThread() const;
    Render::RenderFrameContext MakeFeatureContext(std::uint32_t frameSlot);
    void MaintainRenderFeatures(Render::RenderFrameContext& context);
    void MarkRenderGraphDirty() noexcept { m_RenderGraphDirty = true; }
    Render::RenderThread* m_RenderThread = nullptr;
    std::deque<Render::CommandTicket> m_PendingFrameTickets;
    bool m_RenderGraphDirty = true;
};
} // namespace Aether
