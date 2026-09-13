#pragma once

#include <ImGui/Core/imgui.h>
#include <SDL3/SDL_video.h>
#include <string>

namespace Aether::ImGuiApi
{

struct BorderlessWindowConfig
{
    std::string title;
    int titleBarHeight = 38;
    int resizeBorderWidth = 6;
    int buttonWidth = 46;
    ImVec4 backgroundColor = ImVec4(0.13f, 0.15f, 0.19f, 1.0f);
    ImVec4 closeButtonHoverColor = ImVec4(0.82f, 0.18f, 0.20f, 1.0f);
    ImU32 titleColor = IM_COL32(240, 242, 246, 255);
};

// Attach to the primary ImGui viewport's borderless SDL window on the main thread.
// Draw during an active ImGui frame after the ImGui context is initialized.
// The owner must detach or destroy this object before destroying the SDL window.
class BorderlessWindow
{
public:
    explicit BorderlessWindow(BorderlessWindowConfig config = {});
    ~BorderlessWindow();

    BorderlessWindow(const BorderlessWindow&) = delete;
    BorderlessWindow& operator=(const BorderlessWindow&) = delete;
    BorderlessWindow(BorderlessWindow&&) = delete;
    BorderlessWindow& operator=(BorderlessWindow&&) = delete;

    // Returns false when the platform does not support custom hit testing.
    // The title bar can still be drawn and its buttons remain usable.
    bool Attach(SDL_Window* window);
    void Detach();
    void Draw();

private:
    static SDL_HitTestResult SDLCALL HitTest(SDL_Window* window, const SDL_Point* point, void* data);

    BorderlessWindowConfig m_Config;
    SDL_Window* m_Window = nullptr;
    bool m_HitTestInstalled = false;
    std::string m_ImGuiWindowName;
};

} // namespace Aether::ImGuiApi
