#pragma once
#include "../ImGui/imgui.h"
#include "../ImGui/imgui_impl_glfw.h"
#include "../ImGui/imgui_impl_opengl3.h"
#include "../Render/OpenGLApi.h"
#include <GLFW/glfw3.h>
#include <assert.h>
#include <memory>
#include "../Event/Event.h"
#include <chrono>
#include "Layer.h"
#include "../Core/Input.h"
#include "Aether/Core/Math.h"
namespace Aether
{

class Application
{

public:
    Application();
    ~Application() {};
    void InitWindow();
    void InitGLEW();
    void InitImGui();
    void InitEvent();
    int Run();
    void DispatchEvent();
    void OnUpdate(float ds);
    void OnEvent(Event& e);
    void OnRender();
    void OnImGuiRender();
    void OnDestory();
    void OnLoopBegin();
    void OnLoopEnd();
    void PushLayer(Ref<Layer> layer);
    bool PopLayer(Ref<Layer> layer);
    Vec2i GetWindowSize()
    {
        int width, height;
        glfwGetFramebufferSize(m_Window, &width, &height);
        return Vec2i(width, height);
    }
private:
    //event
    static void WindowSizeCallback(GLFWwindow* window, int width, int height);
    static void WindowFileDropCallback(GLFWwindow* window, int count, const char** paths);
    static void KeyboardEventCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void MouseButtonEventCallback(GLFWwindow* window, int button, int action, int mods);
    static void MouseScrollEventCallback(GLFWwindow* window, double xoffset, double yoffset);
    static std::vector<Event*> s_EventQueue;
    static void ClearEventQueue();
    static void MousePositionEventCallback(GLFWwindow* window, double xpos, double ypos);
private:
    //update
    size_t m_TimeLastTickBegin;
private:
    GLFWwindow* m_Window;
  
private:
    std::vector<Ref<Layer>> m_Layers;
public:
    static void Init(){s_Instance=new Application;}
    static void Release(){delete s_Instance;s_Instance=nullptr;}
    static Application& Get(){return *s_Instance;}
private:
    static Application* s_Instance;
    ImGuiContext* m_ImGuiContext;
};
}//namespace Aether
