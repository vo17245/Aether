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
#include "Layer.h"

AETHER_NAMESPACE_BEGIN
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

};

AETHER_NAMESPACE_END