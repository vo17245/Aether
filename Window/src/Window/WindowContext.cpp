#include "WindowContext.h"
#include "Core/Math.h"
#include "MouseEvent.h"
#include "Window/Event.h"
#include "Window/Window.h"
#include "Render/Vulkan/GlobalRenderContext.h"
namespace Aether {
void WindowContext::WindowResizeCallback(GLFWwindow* window, int width, int height)
{
    auto& windows = Get().m_Windows;
    auto iter = windows.find(window);
    if (iter != windows.end())
    {
        iter->second->PushEvent(WindowResizeEvent(width, height));
    }
    else
    {
        assert(false&&"WindowResizeCallback: window not found");
    }
}

void WindowContext::KeyboardEventCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    Event event;
    if (action == GLFW_RELEASE)
    {
        event = KeyboardReleaseEvent(static_cast<KeyboardCode>(key));
    }
    else if (action == GLFW_PRESS)
    {
        event = KeyboardPressEvent(static_cast<KeyboardCode>(key));
    }
    else if (action == GLFW_REPEAT)
    {
        event = KeyboardRepeatEvent(static_cast<KeyboardCode>(key));
    }
    else
    {
        assert(false&&"KeyboardEventCallback: unknown action");
    }

    auto& windows = Get().m_Windows;
    auto iter = windows.find(window);
    if (iter != windows.end())
    {
        iter->second->PushEvent(event);
    }
    else
    {
        assert(false&&"KeyboardEventCallback: window not found");
    }
}

void WindowContext::FramebufferResizeCallback(GLFWwindow* window, int width, int height)
{
    auto& windows = Get().m_Windows;
    auto iter = windows.find(window);
    if (iter == windows.end())
    {
        assert(false&&"FramebufferResizeCallback: window not found");
        return;
    }
    // push event
    iter->second->PushEvent(FrameBufferResizeEvent(width, height));
    // resize swapchain
    if (width == 0 || height == 0)
    {
        return;
    }

    vkDeviceWaitIdle(vk::GRC::GetDevice());
    iter->second->ReleaseRenderObject();
    iter->second->CreateRenderObject();
    iter->second->m_SwapChainExtent = {static_cast<uint32_t>(width), static_cast<uint32_t>(height)};
}
void WindowContext::CharacterCallback(GLFWwindow* window, unsigned int codepoint)
{
    auto& windows = Get().m_Windows;
    auto iter = windows.find(window);
    if (iter != windows.end())
    {
        Event event = CharacterInputEvent(codepoint);
        iter->second->PushEvent(event);
    }
    else
    {
        assert(false&&"CharacterCallback: window not found");
    }
}
void WindowContext::MousePositionCallback(GLFWwindow* window, double xpos, double ypos)
{
    auto& windows = Get().m_Windows;
    auto iter = windows.find(window);
    if (iter != windows.end())
    {
        Event event = MousePositionEvent(Vec2f(xpos, ypos));
        iter->second->PushEvent(event);
    }
    else
    {
        assert(false&&"MousePositionCallback: window not found");
    }
}
} // namespace Aether