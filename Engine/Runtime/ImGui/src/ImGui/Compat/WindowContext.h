#pragma once
#include "ImGui/Backend/imgui_impl_glfw.h"
#include "ImGui/Backend/imgui_impl_vulkan.h"
namespace Aether::ImGuiApi
{
    struct WindowContext
    {
        ImGui_ImplVulkanH_Window window;
    };
}