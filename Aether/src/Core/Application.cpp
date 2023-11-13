#include "Application.h"

#include <iostream>

#include "../ImGui/imgui.h"
#include "../ImGui/imgui_impl_glfw.h"
#include "../ImGui/imgui_impl_opengl3.h"
#include "../Render/VertexBufferLayout.h"

#include "../Render/Camera.h"
#include "../Event/WindowEvent.h"
#include "../Event/KeyboardEvent.h"
#include "../Event/MouseEvent.h"
#include "Config.h"

#include "../Core/Log.h"
AETHER_NAMESPACE_BEGIN
std::vector<Event*> Application::s_EventQueue;
Application::Application()
{
    InitWindow();
    InitGLEW();
    InitImGui();
    InitEvent();
    Log::Get();
}



void Application::InitWindow()
{
    /* Initialize the library */
    if (!glfwInit())
    {
        Log::Critical("glfwInit failed");
        exit(-1);
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    /* Create a windowed mode window and its OpenGL context */
    m_Window = glfwCreateWindow(1600, 900, GetConfig().app_name, NULL, NULL);
    if (!m_Window)
    {
        glfwTerminate();
        Log::Critical("glfw create window failed");
        exit(-1);
    }

    /* Make the window's context current */
    glfwMakeContextCurrent(m_Window);
}
void Application::InitGLEW()
{
    GLenum err = glewInit();
    if (GLEW_OK != err)
    {
        /* Problem: glewInit failed, something is seriously wrong. */
        std::string errStr((const char*)glewGetErrorString(err));
        Log::Critical("Error: {0}", errStr);
        exit(-1);
    }
}
void Application::InitImGui()
{
    // init imgui
    IMGUI_CHECKVERSION();
    ImGuiContext* imguiContext = ImGui::CreateContext();
    ImGui::SetCurrentContext(imguiContext);
    ImGuiIO& io = ImGui::GetIO(); 
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(m_Window, true);
    ImGui_ImplOpenGL3_Init("#version 460 core");
}

void Application::InitEvent()
{
    glfwSetDropCallback(m_Window, WindowFileDropCallback);
    glfwSetWindowSizeCallback(m_Window, WindowSizeCallback);
    glfwSetKeyCallback(m_Window, KeyboardEventCallback);
    glfwSetMouseButtonCallback(m_Window, MouseButtonEventCallback);
    glfwSetScrollCallback(m_Window, MouseScrollEventCallback);
    glfwSetCursorPosCallback(m_Window, MousePositionEventCallback);
}




int Application::Run()
{
    //viewport init
    OpenGLApi::SetViewport(0, 0, 1600, 900);
    //draw
    OpenGLApi::SetClearColor(0.4f, 0.5f, 0.6f, 1.0f);
    OpenGLApi::EnableDepthTest();
    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(m_Window))
    {
        OnLoopBegin();
        /*Record Time*/
        auto begin = std::chrono::high_resolution_clock::now();
        size_t t= std::chrono::time_point_cast<std::chrono::nanoseconds>(begin).time_since_epoch().count();
        float ds = float(t - m_TimeLastTickBegin) / 1000000000;//nanoSec to sec 10^9
        m_TimeLastTickBegin = t;
         
        /* Render here */
        OpenGLApi::ClearColorBuffer();
        OpenGLApi::ClearDepthBuffer();
        OnRender();
        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        OnImGuiRender();
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        /* Swap front and back buffers */
        glfwSwapBuffers(m_Window);

        /* Poll for and process events */
        ClearEventQueue();
        glfwPollEvents();
        DispatchEvent();
        /* OnUpdate*/
       
        OnUpdate(ds);
        OnLoopEnd();
    }
    // Release resource before window destory
    OnDestory();
    //destory window
    glfwDestroyWindow(m_Window);
    glfwTerminate();
    return 0;
}

void Application::DispatchEvent()
{
    for (auto& eventPointer : s_EventQueue)
    {
        OnEvent(*eventPointer);
    }
}



void Application::WindowSizeCallback(GLFWwindow* window, int width, int height)
{
    s_EventQueue.push_back(new WindowResizeEvent(width, height));
}


void Application::WindowFileDropCallback(GLFWwindow* window, int count, const char** paths)
{
    WindowFileDropEvent* event=new WindowFileDropEvent();
    for (size_t i = 0; i < count; i++)
    {
        event->AddPath(paths[i]);
    }
    s_EventQueue.push_back(event);
}

void Application::KeyboardEventCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    
    if (action == GLFW_RELEASE)
    {
        s_EventQueue.push_back(new KeyboardReleaseEvent( static_cast<KeyboardCode>(key)));
    }
    else if (action == GLFW_PRESS)
    {
        s_EventQueue.push_back(new KeyboardPressEvent(static_cast<KeyboardCode>(key)));
    }
    else if (action == GLFW_REPEAT)
    {
        s_EventQueue.push_back(new KeyboardRepeatEvent(static_cast<KeyboardCode>(key)));
    }
    else
    {
        s_EventQueue.push_back(new Event(EventType::UNKNOWN_EVENT));
    }

}

void Application::MouseButtonEventCallback(GLFWwindow* window, int button, int action, int mods)
{
    ImGui_ImplGlfw_MouseButtonCallback(window, button, action, mods);
    if (action == GLFW_RELEASE)
    {
        s_EventQueue.push_back(new MouseButtonReleaseEvent(static_cast<MouseButtonCode>(button)));
    }
    else if (action == GLFW_PRESS)
    {
        s_EventQueue.push_back(new MouseButtonPressEvent(static_cast<MouseButtonCode>(button)));
    }
    else
    {
        s_EventQueue.push_back(new Event(EventType::UNKNOWN_EVENT));
    }
}

void Application::MouseScrollEventCallback(GLFWwindow* window, double xoffset, double yoffset)
{
    ImGui_ImplGlfw_ScrollCallback(window, xoffset, yoffset);
    s_EventQueue.push_back(new MouseScrollEvent(static_cast<float>(xoffset), static_cast<float>(yoffset)));
}

void Application::ClearEventQueue()
{
    for (auto& eventPointer : s_EventQueue)
    {
        delete eventPointer;
    }
    s_EventQueue.clear();
}
void Application::MousePositionEventCallback(GLFWwindow* window, double xpos, double ypos)
{
    ImGui_ImplGlfw_CursorPosCallback(window,  xpos,  ypos);
    s_EventQueue.push_back(new MousePositionEvent(xpos, ypos));
}
void Application::OnUpdate(float ds)
{
    for (auto& layer : m_Layers)
    {
        layer->OnUpdate(ds);
    }
}
void Application::OnEvent(Event& e)
{
    for (auto& layer : m_Layers)
    {
        layer->OnEvent(e);
    }
}
void Application::OnRender()
{
    for (auto& layer : m_Layers)
    {
        layer->OnRender();
    }
}
void Application::OnImGuiRender()
{
    for (auto& layer : m_Layers)
    {
        layer->OnImGuiRender();
    }
}
void Application::OnDestory()
{
    m_Layers.clear();
}
void Application::OnLoopBegin()
{
    for (auto& layer : m_Layers)
    {
        layer->OnLoopBegin();
    }
}
void Application::OnLoopEnd()
{
    for (auto& layer : m_Layers)
    {
        layer->OnLoopEnd();
    }
}
void Application::PushLayer(Ref<Layer> layer)
{
    m_Layers.push_back(layer);
}
bool Application::PopLayer(Ref<Layer> layer)
{
    for (auto iter = m_Layers.begin();iter != m_Layers.end();iter++)
    {
        if (*iter == layer)
        {
            m_Layers.erase(iter);
            return true;
        }
    }
    return false;
}
AETHER_NAMESPACE_END