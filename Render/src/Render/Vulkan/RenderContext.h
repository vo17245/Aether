#pragma once
#include <memory>
#include <vector>
#include <optional>
#include <string>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include "VulkanUtils.h"
#include "Queue.h"
#include <iostream>
namespace Aether
{
static_assert(std::is_same_v<decltype(nullptr), decltype(VK_NULL_HANDLE)>);
class Window;

namespace vk
{

class RenderContext
{
public:
    struct Config
    {
        bool enableValidationLayers = false;
        bool enableDynamicRendering = false;
    };
private:
    Config m_Config;

private:
    const std::vector<const char*> validationLayers = {"VK_LAYER_KHRONOS_validation"};
#ifdef __APPLE__
    const std::vector<const char*> deviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME,
                                                       VK_KHR_MAINTENANCE1_EXTENSION_NAME, "VK_KHR_portability_subset",
                                                       VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME};
#else
    const std::vector<const char*> deviceExtensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME, VK_KHR_MAINTENANCE1_EXTENSION_NAME, VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME};
#endif

public:
    void Init(Window* window,const Config& config);

public:
    Window* m_Window = nullptr;

    VkInstance m_Instance;
    VkDebugUtilsMessengerEXT m_DebugMessenger;

    VkPhysicalDevice m_PhysicalDevice = VK_NULL_HANDLE;
    VkDevice m_Device;

    Queue m_GraphicsQueue;
    Queue m_PresentQueue;
    // VkCommandPool m_GraphicsCommandPool;
    QueueFamilyIndices m_QueueFamilyIndices;

    void InitVulkan();

    void Cleanup();

    void CreateInstance();

    void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);

    void SetupDebugMessenger();

    void PickPhysicalDevice();

    void CreateLogicalDevice();

    // void CreateCommandPool();

    std::vector<const char*> GetRequiredExtensions();

    bool CheckValidationLayerSupport();

    static std::vector<char> ReadFile(const std::string& filename);

    static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                        VkDebugUtilsMessageTypeFlagsEXT messageType,
                                                        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                                                        void* pUserData);

    VkPhysicalDevice ChoosePhysicalDevice(const std::vector<VkPhysicalDevice>& devices);
};

} // namespace vk
} // namespace Aether