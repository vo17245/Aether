#include "ImGuiApi.h"
#include "Backend/imgui_impl_vulkan.h"
#include "Backend/imgui_impl_glfw.h"
namespace Aether::ImGui
{
    void NewFrame()
    {
        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
    }
}