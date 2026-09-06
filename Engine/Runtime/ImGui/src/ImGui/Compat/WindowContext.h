#pragma once
#include "ImGui/Backend/imgui_impl_sdl3.h"
#include "ImGui/Backend/imgui_impl_vulkan.h"
namespace Aether::ImGuiApi
{
    struct WindowContext
    {
        ImGui_ImplVulkanH_Window window;
    };
}