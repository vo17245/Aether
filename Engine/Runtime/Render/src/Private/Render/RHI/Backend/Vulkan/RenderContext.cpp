#include "Render/RHI/Backend/Vulkan/RenderContext.h"
#include <cassert>
#include "Core/Math.h"
#include "Render/RHI/Backend/Vulkan/VulkanUtils.h"
#include <iostream>
#include <fstream>
#include <memory>
#include <stdexcept>
#include <algorithm>
#include <vector>
#include <cstring>
#include <cstdlib>
#include <cstdint>
#include <limits>
#include <optional>
#include <set>
#include "Render/RHI/Backend/Vulkan/FrameBuffer.h"
#include "Render/RHI/Backend/Vulkan/ImageView.h"
#include "Render/RHI/Backend/Vulkan/Semaphore.h"
#include "Render/RHI/Backend/Vulkan/Fence.h"
#include "vulkan/vulkan_core.h"
#include "Render/RHI/Backend/Vulkan/Queue.h"
#include <print>
#include "Render/Config.h"

namespace Aether
{
namespace vk
{

void RenderContext::Init(const InitResource& resource,const Config& config)
{
    m_Config=config;
    InitVulkan(resource);
}

void RenderContext::InitVulkan(const InitResource& resource)
{
    CreateInstance();
    SetupDebugMessenger();
    PickPhysicalDevice(resource);
    CreateLogicalDevice(resource);
    m_QueueFamilyIndices = findQueueFamilies(m_PhysicalDevice, resource.surface);
}

void RenderContext::Cleanup()
{
    // vkDestroyCommandPool(m_Device, m_GraphicsCommandPool, nullptr);
    // delete m_Window;
    vkDestroyDevice(m_Device, nullptr);

    if (m_Config.enableValidationLayers)
    {
        DestroyDebugUtilsMessengerEXT(m_Instance, m_DebugMessenger, nullptr);
    }

    vkDestroyInstance(m_Instance, nullptr);

    // WindowContext::Shutdown();
}

void RenderContext::CreateInstance()
{
    if (m_Config.enableValidationLayers && !CheckValidationLayerSupport())
    {
        assert(false&& "validation layers requested, but not available!");
    }

    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Hello Triangle";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = Render::Config::VulkanApiVersion;

    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;
#ifdef __APPLE__
// createInfo.flags=VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
#endif
    auto extensions = GetRequiredExtensions();
    createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();
    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
    if (m_Config.enableValidationLayers)
    {
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();

        PopulateDebugMessengerCreateInfo(debugCreateInfo);
        createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
    }
    else
    {
        createInfo.enabledLayerCount = 0;
        createInfo.pNext = nullptr;
    }
    VkResult result = vkCreateInstance(&createInfo, nullptr, &m_Instance);
    if (result != VK_SUCCESS)
    {
        throw std::runtime_error(
            std::format("failed to create instance! vkCreateInstance return VkResult: {}", int(result)));
    }
}

void RenderContext::PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
{
    createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT
                                 | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
                                 | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
                             | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
                             | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = DebugCallback;
}

void RenderContext::SetupDebugMessenger()
{
    if (!m_Config.enableValidationLayers)
        return;

    VkDebugUtilsMessengerCreateInfoEXT createInfo;
    PopulateDebugMessengerCreateInfo(createInfo);

    if (CreateDebugUtilsMessengerEXT(m_Instance, &createInfo, nullptr, &m_DebugMessenger) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to set up debug messenger!");
    }
}

// void RenderContext::createSurface()
//{
//     if (glfwCreateWindowSurface(instance, window->GetHandle(), nullptr, &surface) != VK_SUCCESS)
//     {
//         throw std::runtime_error("failed to create window surface!");
//     }
// }

void RenderContext::PickPhysicalDevice(const InitResource& resource)
{
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(m_Instance, &deviceCount, nullptr);

    if (deviceCount == 0)
    {
        throw std::runtime_error("failed to find GPUs with Vulkan support!");
    }

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(m_Instance, &deviceCount, devices.data());
    std::vector<VkPhysicalDevice> suitableDevices;
    for (const auto& device : devices)
    {
        if (isDeviceSuitable(device, resource.surface))
        {
            suitableDevices.push_back(device);
        }
    }

    if (suitableDevices.empty())
    {
        throw std::runtime_error("failed to find a suitable GPU!");
    }
    m_PhysicalDevice = ChoosePhysicalDevice(suitableDevices);
}

void RenderContext::CreateLogicalDevice(const InitResource& resource)
{
    // 1. Dynamic Rendering feature
    VkPhysicalDeviceDynamicRenderingFeaturesKHR dynamicRenderingFeature{};
    dynamicRenderingFeature.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES_KHR;
    dynamicRenderingFeature.dynamicRendering = VK_TRUE;

    // 2. Device features (sampler anisotropy)
    VkPhysicalDeviceFeatures deviceFeatures{};
    deviceFeatures.samplerAnisotropy = VK_TRUE;

    VkPhysicalDeviceFeatures2 deviceFeatures2{};
    deviceFeatures2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
    deviceFeatures2.features = deviceFeatures;
    if (m_Config.enableDynamicRendering)
    {
        deviceFeatures2.pNext = &dynamicRenderingFeature;
    }
    //===============
    QueueFamilyIndices indices = findQueueFamilies(m_PhysicalDevice, resource.surface);

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t> uniqueQueueFamilies = {indices.graphicsFamily.value(), indices.presentFamily.value()};

    float queuePriority = 1.0f;
    for (uint32_t queueFamily : uniqueQueueFamilies)
    {
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }

    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.pNext = &deviceFeatures2;

    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pQueueCreateInfos = queueCreateInfos.data();

    createInfo.pEnabledFeatures = nullptr;

    createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
    createInfo.ppEnabledExtensionNames = deviceExtensions.data();

    if (m_Config.enableValidationLayers)
    {
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();
    }
    else
    {
        createInfo.enabledLayerCount = 0;
        createInfo.ppEnabledLayerNames = nullptr;
    }

    if (vkCreateDevice(m_PhysicalDevice, &createInfo, nullptr, &m_Device) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create logical device!");
    }
    VkQueue graphicsQueue;
    VkQueue presentsQueue;
    vkGetDeviceQueue(m_Device, indices.graphicsFamily.value(), 0, &graphicsQueue);
    vkGetDeviceQueue(m_Device, indices.presentFamily.value(), 0, &presentsQueue);
    m_GraphicsQueue.m_Handle = graphicsQueue;
    m_GraphicsQueue.m_Type = Queue::Type::Graphics;
    m_PresentQueue.m_Handle = presentsQueue;
    m_PresentQueue.m_Type = Queue::Type::Present;
}

// void RenderContext::CreateCommandPool()
//{
//
//     VkCommandPoolCreateInfo poolInfo{};
//     poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
//     poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
//     poolInfo.queueFamilyIndex = m_QueueFamilyIndices.graphicsFamily.value();
//
//     if (vkCreateCommandPool(m_Device, &poolInfo, nullptr, &m_GraphicsCommandPool) != VK_SUCCESS)
//     {
//         throw std::runtime_error("failed to create command pool!");
//     }
// }

std::vector<const char*> RenderContext::GetRequiredExtensions()
{
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

    if (m_Config.enableValidationLayers)
    {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }
#ifdef __APPLE__
    // extensions.push_back("VK_KHR_portability_subset");
    extensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
// extensions.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
#endif

    return extensions;
}

bool RenderContext::CheckValidationLayerSupport()
{
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    for (const char* layerName : validationLayers)
    {
        bool layerFound = false;

        for (const auto& layerProperties : availableLayers)
        {
            if (strcmp(layerName, layerProperties.layerName) == 0)
            {
                layerFound = true;
                break;
            }
        }

        if (!layerFound)
        {
            return false;
        }
    }

    return true;
}

std::vector<char> RenderContext::ReadFile(const std::string& filename)
{
    std::ifstream file(filename, std::ios::ate | std::ios::binary);

    if (!file.is_open())
    {
        throw std::runtime_error("failed to open file!");
    }

    size_t fileSize = (size_t)file.tellg();
    std::vector<char> buffer(fileSize);

    file.seekg(0);
    file.read(buffer.data(), fileSize);

    file.close();

    return buffer;
}

VKAPI_ATTR VkBool32 VKAPI_CALL RenderContext::DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                            VkDebugUtilsMessageTypeFlagsEXT messageType,
                                                            const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                                                            void* pUserData)
{
    if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
    {
        std::println("validation layer: {}", pCallbackData->pMessage);
    }
    else
    {
        std::println("validation layer: {0}", pCallbackData->pMessage);
    }
    return VK_FALSE;
}
VkPhysicalDevice RenderContext::ChoosePhysicalDevice(const std::vector<VkPhysicalDevice>& devices)
{
    std::vector<std::string> modelName;
    for (const auto& device : devices)
    {
        VkPhysicalDeviceProperties deviceProperties;
        vkGetPhysicalDeviceProperties(device, &deviceProperties);

        modelName.emplace_back(deviceProperties.deviceName);
    }
    for (auto& name : modelName)
    {
        // 转小写
        std::transform(name.begin(), name.end(), name.begin(), ::tolower);
    }
    // nvida?
    for (size_t i = 0; i < modelName.size(); i++)
    {
        auto pos = modelName[i].find("nvidia");
        if (pos != std::string::npos)
        {
            return devices[i];
        }
    }
    // amd ?
    for (size_t i = 0; i < modelName.size(); i++)
    {
        auto pos = modelName[i].find("amd");
        if (pos != std::string::npos)
        {
            return devices[i];
        }
    }
    // intel ?
    for (size_t i = 0; i < modelName.size(); i++)
    {
        auto pos = modelName[i].find("intel");
        if (pos != std::string::npos)
        {
            return devices[i];
        }
    }

    return devices[0];
}
} // namespace vk
} // namespace Aether