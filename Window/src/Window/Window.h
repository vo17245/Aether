#pragma once
#include "Render/RenderApi/DeviceTexture.h"
#include "Render/Vulkan/DescriptorPool.h"
#include "Render/Vulkan/Fence.h"
#include "Render/Vulkan/GraphicsCommandBuffer.h"
#include "Render/Vulkan/RenderPass.h"
#include "Render/Vulkan/Semaphore.h"
#include "Render/Vulkan/Texture2D.h"
#include "vulkan/vulkan_core.h"
#include <memory>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vector>
#include "Event.h"
#include "Layer.h"
#include "Render/Vulkan/FrameBuffer.h"
#include "Render/Vulkan/ImageView.h"
#include "Input.h"
#include "GammaFilter.h"
#include "Render/RenderGraph/RenderGraph.h"
namespace Aether {
namespace vk {
class RenderContext;

}
class Window
{
    friend class WindowContext;
    friend class vk::RenderContext;

public:
    constexpr const static inline int MAX_FRAMES_IN_FLIGHT = 2;

public:
    ~Window();
    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;
    Window(Window&& other) noexcept;
    Window& operator=(Window&& other) noexcept;
    GLFWwindow* GetHandle() const;
    bool ShouldClose() const;
    void DispatchEvent();
    /**
     * @brief Create a window
     * @return pointer to the window, nullptr if failed
     */
    static Window* Create(int width, int height, const char* title);
    /**
     * @note
     *   只是把layer挂在到window，不转移所有权
     */
    void PushLayer(Layer* layer);
    void PushLayers(const std::span<Layer*>& layers);
    void PopLayer(Layer* layer);
    VkSwapchainKHR GetSwapchain() const;
    const std::vector<VkImage>& GetImages() const;
    std::vector<VkImage>& GetImages();
    const std::vector<vk::ImageView>& GetImageViews() const;
    std::vector<vk::ImageView>& GetImageViews();
    bool CreateRenderObject();
    void ReleaseRenderObject();
    bool CreateSyncObjects();
    bool ReleaseSyncObjects();
    bool ReleaseVulkanObjects();
    VkSurfaceKHR GetSurface() const;
    Vec2i GetSize() const;
    void OnUpdate(float sec);
    void OnRender();
    void PushEvent(const Event& e);
    Input& GetInput();

    void ReleaseFinalImage();
    bool CreateFinalImage();
    DeviceTexture& GetFinalTexture(uint32_t index);
    void SetSize(uint32_t width, uint32_t height);
    uint32_t GetCurrentFrameIndex()
    {
        return m_CurrentFrame;
    }
    RenderGraph::AccessId<DeviceTexture> GetFinalImageAccessId() const
    {
        return m_FinalImageAccessId;
    }
    DeviceDescriptorPool& GetCurrentDescriptorPool()
    {
        return m_DescriptorPools[m_CurrentFrame];
    }
    // create resource arena and lru pool
    void InitRenderGraphResource();
    inline RenderGraph::ResourceArena& GetResourceArena()
    {
        return *m_ResourceArena;
    }
    inline RenderGraph::ResourceLruPool& GetResourcePool()
    {
        return *m_ResourcePool;
    }
    inline RenderGraph::RenderGraph& GetRenderGraph()
    {
        return *m_RenderGraph;
    }
private:
    std::vector<Event> m_Event;
    std::vector<Layer*> m_Layers;
    VkSwapchainKHR m_SwapChain = VK_NULL_HANDLE;
    std::vector<VkImage> m_SwapChainImages;
    std::vector<vk::ImageView> m_SwapChainImageViews;
    VkFormat m_SwapChainImageFormat{};
    VkExtent2D m_SwapChainExtent{};
    VkSurfaceKHR m_Surface = VK_NULL_HANDLE;
    GLFWwindow* m_Handle=nullptr;
    std::unique_ptr<vk::Semaphore> m_ImageAvailableSemaphore[MAX_FRAMES_IN_FLIGHT];
    std::unique_ptr<vk::Semaphore> m_RenderFinishedSemaphore[MAX_FRAMES_IN_FLIGHT];
    Scope<vk::Fence> m_CommandBufferFences[MAX_FRAMES_IN_FLIGHT];
    DeviceCommandBuffer m_GraphicsCommandBuffer[MAX_FRAMES_IN_FLIGHT];
    //=========== final image
    DeviceTexture m_FinalTextures[MAX_FRAMES_IN_FLIGHT];
    DeviceFrameBuffer m_TonemapFrameBuffers[MAX_FRAMES_IN_FLIGHT];
    DeviceRenderPass m_TonemapRenderPass; 
    Scope<WindowInternal::GammaFilter> m_GammaFilter;
    DeviceDescriptorPool m_DescriptorPools[MAX_FRAMES_IN_FLIGHT];
    //================================
    uint32_t m_CurrentFrame=0;
private:

    /**
     * @brief create surface
     */
    VkResult CreateSurface(VkInstance instance);
    Window(GLFWwindow* window);
    /**
     *@brief Create a glfw window handle
     */
    static GLFWwindow* CreateGlfwHandle(int width, int height, const char* title);
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
private:
    Input m_Input;
private://render graph
    Scope<RenderGraph::ResourceArena> m_ResourceArena;
    Scope<RenderGraph::ResourceLruPool> m_ResourcePool;
    Scope<RenderGraph::RenderGraph> m_RenderGraph;
    
    // create render graph, register final image
    // and call each layer RegisterRenderPasses function
    void CreateRenderGraph();
    RenderGraph::AccessId<DeviceTexture> m_FinalImageAccessId;

private:
    void ImGuiRecordCommandBuffer(DeviceCommandBuffer& commandBuffer);
};
} // namespace Aether