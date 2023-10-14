#pragma once
#include "../ImGui/imgui.h"
#include "../ImGui/imgui_impl_glfw.h"
#include "../ImGui/imgui_impl_opengl3.h"
#include "../Render/OpenGLApi.h"
#include <GLFW/glfw3.h>
#include <assert.h>
#include "../Render/Shader.h"
#include <memory>
#include "../Render/VertexArray.h"
#include "../Render/VertexBuffer.h"
#include "../Render/IndexBuffer.h"
#include "../Render/Texture2D.h"
#include "../Event/Event.h"
#include <chrono>

AETHER_NAMESPACE_BEGIN
class Application
{
public:
    Application();
    virtual ~Application();
    void InitWindow();
    void InitGLEW();
    void InitImGui();
    void InitEvent();
    void Run();
    void DispatchEvent();
    virtual void OnUpdate(float sec) {}
    virtual void OnEvent(Event& event) {}
    virtual void OnRender() {}
    virtual void OnImGuiRender() {}
    virtual void OnDestory(){}
private:
    //event
    static void WindowSizeCallback(GLFWwindow* window, int width, int height);
    static void WindowFileDropCallback(GLFWwindow* window, int count, const char** paths);
    static void KeyboardEventCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void MouseButtonEventCallback(GLFWwindow* window, int button, int action, int mods);
    static void MouseScrollEventCallback(GLFWwindow* window, double xoffset, double yoffset);
    static std::vector<Event*> s_EventQueue;
    static void ClearEventQueue();
private:
    //update
    std::chrono::nanoseconds m_TimeStamp;
private:
    GLFWwindow* m_Window;

};
AETHER_NAMESPACE_END