#include "ImGuiApi.h"
#include "ImGui/Backend/imgui_impl_vulkan.h"
#include "ImGui/Backend/imgui_impl_glfw.h"
#include <Window/Window.h>
#include <Render/Vulkan/GlobalPipelineCache.h>
namespace Aether::ImGuiApi
{
static VkDescriptorPool g_DescriptorPool = VK_NULL_HANDLE;
static ImGuiContext* g_MainContext = nullptr;
void NewFrame()
{
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}
void Shutdown()
{
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    vkDestroyDescriptorPool(vk::GRC::GetDevice(), g_DescriptorPool, nullptr);
}
static void check_vk_result(VkResult err)
{
    if (err == VK_SUCCESS)
        return;
    fprintf(stderr, "[vulkan] Error: VkResult = %d\n", err);
    if (err < 0)
        abort();
}
void Init(Window& window)
{
    // Create Descriptor Pool
    // If you wish to load e.g. additional textures you may need to alter pools sizes and maxSets.
    {
        VkDescriptorPoolSize pool_sizes[] = {
            {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, IMGUI_IMPL_VULKAN_MINIMUM_IMAGE_SAMPLER_POOL_SIZE},
        };
        VkDescriptorPoolCreateInfo pool_info = {};
        pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
        pool_info.maxSets = 0;
        for (VkDescriptorPoolSize& pool_size : pool_sizes)
            pool_info.maxSets += pool_size.descriptorCount;
        pool_info.poolSizeCount = (uint32_t)IM_ARRAYSIZE(pool_sizes);
        pool_info.pPoolSizes = pool_sizes;
        auto err = vkCreateDescriptorPool(vk::GRC::GetDevice(), &pool_info, nullptr, &g_DescriptorPool);
        check_vk_result(err);
    }

    // Create Framebuffers
    int w, h;
    glfwGetFramebufferSize(window.GetHandle(), &w, &h);
    ImGui_ImplVulkanH_Window* wd = &window.GetImGuiContext().window;

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    g_MainContext=ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;  // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    // ImGui::StyleColorsLight();

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForVulkan(window.GetHandle(), true);
    ImGui_ImplVulkan_InitInfo init_info = {};
    // init_info.ApiVersion = VK_API_VERSION_1_3;              // Pass in your value of VkApplicationInfo::apiVersion,
    // otherwise will default to header version.
    init_info.Instance = vk::GRC::GetInstance();
    init_info.PhysicalDevice = vk::GRC::GetPhysicalDevice();
    init_info.Device = vk::GRC::GetDevice();
    init_info.QueueFamily = vk::GRC::GetQueueFamilyIndices().graphicsFamily.value();
    init_info.Queue = vk::GRC::GetGraphicsQueue().GetHandle();
    init_info.PipelineCache = vk::GlobalPipelineCache::Get();
    init_info.DescriptorPool = g_DescriptorPool;
    init_info.RenderPass = wd->RenderPass;
    init_info.Subpass = 0;
    init_info.MinImageCount = window.SwapChainImageCount();
    init_info.ImageCount = window.SwapChainImageCount();
    init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
    init_info.Allocator = nullptr;
    init_info.CheckVkResultFn = check_vk_result;
    ImGui_ImplVulkan_Init(&init_info);
}
void EnableDocking()
{
    ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;
}
} // namespace Aether::ImGuiApi