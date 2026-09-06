#include "ImGuiApi.h"
#include "ImGui/Backend/imgui_impl_rendergraph.h"
#include "ImGui/Backend/imgui_impl_sdl3.h"
#include <Window/Window.h>
#include <stdexcept>

namespace Aether::ImGuiApi
{
static ImGuiContext* g_MainContext = nullptr;
void NewFrame()
{
    ImGui_ImplRenderGraph_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();
}
void Shutdown()
{
    ImGui_ImplRenderGraph_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();
    g_MainContext = nullptr;
}
void Init(Window& window)
{
    IMGUI_CHECKVERSION();
    g_MainContext = ImGui::CreateContext();
    auto& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard | ImGuiConfigFlags_NavEnableGamepad;
    ImGui::StyleColorsDark();
    if (!ImGui_ImplSDL3_InitForVulkan(window.GetHandle()))
    {
        ImGui::DestroyContext();
        g_MainContext = nullptr;
        throw std::runtime_error("Failed to initialize ImGui SDL3 backend");
    }
    if (!ImGui_ImplRenderGraph_Init())
    {
        ImGui_ImplSDL3_Shutdown();
        ImGui::DestroyContext();
        g_MainContext = nullptr;
        throw std::runtime_error("Failed to initialize ImGui RenderGraph backend");
    }
}
void EnableDocking()
{
    ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;
}
void SwitchToMainContext()
{
    ImGui::SetCurrentContext(g_MainContext);
}
} // namespace Aether::ImGuiApi
