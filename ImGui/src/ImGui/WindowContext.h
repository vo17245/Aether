#pragma once
#include "Backend/imgui_impl_glfw.h"
#include "Backend/imgui_impl_vulkan.h"
namespace Aether::ImGuiApi
{
    struct WindowContext
    {
        ImGui_ImplVulkanH_Window window;
    };
}