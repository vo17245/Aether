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
    InitFrontend(window);
    if (!ImGui_ImplRenderGraph_Init())
    {
        ShutdownFrontend();
        throw std::runtime_error("Failed to initialize ImGui RenderGraph backend");
    }
}
void InitFrontend(Window& window)
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
    io.BackendRendererName = "imgui_impl_rendergraph_packet";
    io.BackendFlags |= ImGuiBackendFlags_RendererHasVtxOffset | ImGuiBackendFlags_RendererHasTextures;
}
void PrepareRendererShutdown()
{
    if (!g_MainContext) return;
    for (ImTextureData* texture : ImGui::GetPlatformIO().Textures)
    {
        texture->SetTexID(ImTextureID_Invalid);
        texture->BackendUserData = nullptr;
        texture->SetStatus(ImTextureStatus_Destroyed);
    }
}
void ShutdownFrontend()
{
    if (!g_MainContext) return;
    auto& io = ImGui::GetIO();
    io.BackendRendererUserData = nullptr;
    io.BackendRendererName = nullptr;
    io.BackendFlags &= ~(ImGuiBackendFlags_RendererHasVtxOffset | ImGuiBackendFlags_RendererHasTextures);
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();
    g_MainContext = nullptr;
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
