#include "BorderlessWindow.h"

#include <SDL3/SDL_events.h>
#include <algorithm>
#include <utility>

namespace Aether::ImGuiApi
{

BorderlessWindow::BorderlessWindow(BorderlessWindowConfig config) : m_Config(std::move(config))
{
    m_Config.titleBarHeight = std::max(1, m_Config.titleBarHeight);
    m_Config.resizeBorderWidth = std::max(0, m_Config.resizeBorderWidth);
    m_Config.buttonWidth = std::max(1, m_Config.buttonWidth);
}

BorderlessWindow::~BorderlessWindow()
{
    Detach();
}

bool BorderlessWindow::Attach(SDL_Window* window)
{
    if (window == nullptr)
        return false;
    if (m_Window == window && m_HitTestInstalled)
        return true;

    if (m_Window != window)
    {
        Detach();
        m_Window = window;
        m_ImGuiWindowName = "##AetherBorderlessTitleBar_" + std::to_string(SDL_GetWindowID(window));
    }
    m_HitTestInstalled = SDL_SetWindowHitTest(window, HitTest, this);
    return m_HitTestInstalled;
}

void BorderlessWindow::Detach()
{
    if (m_Window != nullptr && m_HitTestInstalled)
        SDL_SetWindowHitTest(m_Window, nullptr, nullptr);
    m_Window = nullptr;
    m_HitTestInstalled = false;
    m_ImGuiWindowName.clear();
}

SDL_HitTestResult SDLCALL BorderlessWindow::HitTest(SDL_Window* window, const SDL_Point* point, void* data)
{
    const auto& config = static_cast<BorderlessWindow*>(data)->m_Config;
    int width = 0;
    int height = 0;
    SDL_GetWindowSize(window, &width, &height);
    if (width <= 0 || height <= 0)
        return SDL_HITTEST_NORMAL;

    const SDL_WindowFlags flags = SDL_GetWindowFlags(window);
    if ((flags & (SDL_WINDOW_MAXIMIZED | SDL_WINDOW_FULLSCREEN)) == 0 &&
        (flags & SDL_WINDOW_RESIZABLE) != 0)
    {
        const bool left = point->x < config.resizeBorderWidth;
        const bool right = point->x >= width - config.resizeBorderWidth;
        const bool top = point->y < config.resizeBorderWidth;
        const bool bottom = point->y >= height - config.resizeBorderWidth;
        if (top && left) return SDL_HITTEST_RESIZE_TOPLEFT;
        if (top && right) return SDL_HITTEST_RESIZE_TOPRIGHT;
        if (bottom && left) return SDL_HITTEST_RESIZE_BOTTOMLEFT;
        if (bottom && right) return SDL_HITTEST_RESIZE_BOTTOMRIGHT;
        if (left) return SDL_HITTEST_RESIZE_LEFT;
        if (right) return SDL_HITTEST_RESIZE_RIGHT;
        if (top) return SDL_HITTEST_RESIZE_TOP;
        if (bottom) return SDL_HITTEST_RESIZE_BOTTOM;
    }

    if ((flags & SDL_WINDOW_FULLSCREEN) == 0 && point->y >= 0 && point->y < config.titleBarHeight &&
        point->x >= 0 && point->x < width - 3 * config.buttonWidth)
        return SDL_HITTEST_DRAGGABLE;
    return SDL_HITTEST_NORMAL;
}

void BorderlessWindow::Draw()
{
    if (m_Window == nullptr || ImGui::GetCurrentContext() == nullptr)
        return;

    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    const float titleBarHeight = static_cast<float>(m_Config.titleBarHeight);
    const float buttonWidth = static_cast<float>(m_Config.buttonWidth);
    ImGui::SetNextWindowPos(viewport->Pos);
    ImGui::SetNextWindowSize(ImVec2(viewport->Size.x, titleBarHeight));
    ImGui::SetNextWindowViewport(viewport->ID);

    constexpr ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
                                       ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoNav |
                                       ImGuiWindowFlags_NoBringToFrontOnFocus;
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
    ImGui::PushStyleColor(ImGuiCol_WindowBg, m_Config.backgroundColor);
    const bool visible = ImGui::Begin(m_ImGuiWindowName.c_str(), nullptr, flags);
    if (visible)
    {
        const char* title = m_Config.title.empty() ? SDL_GetWindowTitle(m_Window) : m_Config.title.c_str();
        ImGui::GetWindowDrawList()->AddText(
            ImVec2(viewport->Pos.x + 12.0f, viewport->Pos.y + (titleBarHeight - ImGui::GetFontSize()) * 0.5f),
            m_Config.titleColor, title);

        ImGui::SetCursorScreenPos(ImVec2(viewport->Pos.x + viewport->Size.x - 3.0f * buttonWidth,
                                         viewport->Pos.y));
        if (ImGui::Button("-##Minimize", ImVec2(buttonWidth, titleBarHeight)))
            SDL_MinimizeWindow(m_Window);
        ImGui::SameLine();

        const bool maximized = (SDL_GetWindowFlags(m_Window) & SDL_WINDOW_MAXIMIZED) != 0;
        if (ImGui::Button(maximized ? "o##Maximize" : "[]##Maximize",
                          ImVec2(buttonWidth, titleBarHeight)))
        {
            if (maximized)
                SDL_RestoreWindow(m_Window);
            else
                SDL_MaximizeWindow(m_Window);
        }
        ImGui::SameLine();

        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, m_Config.closeButtonHoverColor);
        if (ImGui::Button("X##Close", ImVec2(buttonWidth, titleBarHeight)))
        {
            SDL_Event event{};
            event.type = SDL_EVENT_WINDOW_CLOSE_REQUESTED;
            event.window.windowID = SDL_GetWindowID(m_Window);
            SDL_PushEvent(&event);
        }
        ImGui::PopStyleColor();
    }
    ImGui::End();
    ImGui::PopStyleColor();
    ImGui::PopStyleVar(5);
}

} // namespace Aether::ImGuiApi
