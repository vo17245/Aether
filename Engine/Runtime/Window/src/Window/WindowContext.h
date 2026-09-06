#pragma once

#include <unordered_map>
#include <vector>
#include <Core/Core.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_video.h>

namespace Aether
{
class Window;

class WindowContext
{
public:
    static bool Init();
    static void Shutdown();
    static void PollEvents();
    static void Register(SDL_Window* handle, Window* instance);
    static void Remove(SDL_Window* handle);
    static Vec2i MainMonitorSize();
    static std::vector<const char*> RequiredVulkanInstanceExtensions();

private:
    static WindowContext& Get()
    {
        static WindowContext context;
        return context;
    }

    static Window* FindWindow(SDL_WindowID id);
    static void HandleEvent(const SDL_Event& event);
    static void HandleFramebufferResize(Window& window, int width, int height);

    std::unordered_map<SDL_WindowID, Window*> m_Windows;
};
} // namespace Aether
