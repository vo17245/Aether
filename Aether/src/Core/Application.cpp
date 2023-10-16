#include "Application.h"

#include <iostream>
#include "../Render/Math.h"
#include "../ImGui/imgui.h"
#include "../ImGui/imgui_impl_glfw.h"
#include "../ImGui/imgui_impl_opengl3.h"
#include "../Render/VertexBufferLayout.h"
#include "../Render/Renderer.h"
#include "../Render/Camera.h"
#include "../Event/WindowEvent.h"
#include "../Event/KeyboardEvent.h"
#include "../Event/MouseEvent.h"


#include "../Core/Log.h"
AETHER_NAMESPACE_BEGIN
std::vector<Event*> Application::s_EventQueue;
Application::Application()
{
    InitWindow();
    InitGLEW();
    InitImGui();
    InitEvent();
}

Application::~Application()
{

}

void Application::InitWindow()
{
    /* Initialize the library */
    assert(glfwInit() && "glfwInit failed");
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    /* Create a windowed mode window and its OpenGL context */
    m_Window = glfwCreateWindow(1600, 900, "Aether", NULL, NULL);
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
    ImGuiIO& io = ImGui::GetIO(); //(void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
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
        size_t timestampBegin = std::chrono::time_point_cast<std::chrono::nanoseconds>(begin).time_since_epoch().count();
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
        auto end = std::chrono::high_resolution_clock::now();
        size_t timestampEnd = std::chrono::time_point_cast<std::chrono::nanoseconds>(begin).time_since_epoch().count();
        float deltaSec = float(timestampEnd - timestampBegin) / 1000000000;//nanoSec to sec 10^9
        OnUpdate(deltaSec);
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
    s_EventQueue.push_back(new MouseScrollEvent(xoffset,yoffset));
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
AETHER_NAMESPACE_END