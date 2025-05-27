#pragma once

#include <optional>
#include <unordered_map>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
namespace Aether {
class Window;
class WindowContext
{
public:
    static bool Init()
    {
        return glfwInit();
    }
    static void Shutdown()
    {
        glfwTerminate();
    }
    static void PollEvents()
    {
        glfwPollEvents();
    }
    ~WindowContext() = default;

    static void Register(GLFWwindow* handle, Window* instance)
    {
        Get().m_Windows[handle] = instance;
        InitEvent(handle);
    }
    static void Remove(GLFWwindow* handle)
    {
        Get().m_Windows.erase(handle);
    }

private:
    static GLFWwindow* CreateGlfwHandle(int width, int height, const char* title)
    {
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        GLFWwindow* window = glfwCreateWindow(width, height, title, nullptr, nullptr);
        return window;
    }
    static WindowContext& Get()
    {
        static WindowContext s_Context;
        return s_Context;
    }
    WindowContext() = default;
    std::unordered_map<GLFWwindow*, Window*> m_Windows;

private: // event callback
    static void WindowResizeCallback(GLFWwindow* window, int width, int height);
    // static void WindowFileDropCallback(GLFWwindow* window, int count, const char** paths);
    static void KeyboardEventCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void MouseButtonEventCallback(GLFWwindow* window, int button, int action, int mods);
    // static void MouseScrollEventCallback(GLFWwindow* window, double xoffset, double yoffset);
    static void MousePositionCallback(GLFWwindow* window, double xpos, double ypos);
    static void CharacterCallback(GLFWwindow* window, unsigned int codepoint);
    static void InitEvent(GLFWwindow* window)
    {
        // glfwSetDropCallback(window, WindowFileDropCallback);
        glfwSetWindowSizeCallback(window, WindowResizeCallback);
        glfwSetKeyCallback(window, KeyboardEventCallback);
        glfwSetMouseButtonCallback(window, MouseButtonEventCallback);
        // glfwSetScrollCallback(window, MouseScrollEventCallback);
        // glfwSetCursorPosCallback(window, MousePositionEventCallback);
        glfwSetFramebufferSizeCallback(window, FramebufferResizeCallback);
        glfwSetCharCallback(window, CharacterCallback);
        glfwSetCursorPosCallback(window, MousePositionCallback);
        
    }
    static void FramebufferResizeCallback(GLFWwindow* window, int width, int height);
};
} // namespace Aether