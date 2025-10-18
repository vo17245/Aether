#pragma once
#include <Imgui/Core/imgui.h>
#include <functional>
#include <Window/Window.h>
using namespace Aether;
class MainWindow
{
public:
    void SetOsWindow(Window* window)
    {
        m_OsWindow=window;
    }
    void DrawMainWindowEnd()
    {
        ImGui::End();
    }
    void DrawMainWindowMenuBar()
    {
        if (ImGui::BeginMenuBar())
        {
            if (ImGui::BeginMenu("File"))
            {
                if (ImGui::MenuItem("Open", "Ctrl+O")) { /* 打开文件逻辑 */ }
                if (ImGui::MenuItem("Save", "Ctrl+S")) { /* 保存逻辑 */ }
                ImGui::Separator();
                if (ImGui::MenuItem("Exit")) { /* 退出逻辑 */ }
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Edit"))
            {
                if (ImGui::MenuItem("Undo", "Ctrl+Z")) {}
                if (ImGui::MenuItem("Redo", "Ctrl+Y", false, false)) {} // 禁用项
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Window"))
            {
                if (ImGui::MenuItem("Full Screen", "F11"))
                {
                    if (m_OnMaximize)
                    {
                        m_OnMaximize();
                    }
                }
                ImGui::EndMenu();
            }

            ImGui::EndMenuBar();
        }
    }
    void DrawMainWindowBegin()
    {
        Vec2i size = m_OsWindow->GetSize();
        // full screen
        ImGui::SetNextWindowSize(ImVec2((float)size.x(), (float)size.y()));
        // no title bar
        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoNav
                                        | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoResize
                                        | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_MenuBar;
        ImGui::PushStyleColor(ImGuiCol_TitleBg, ImGui::GetStyleColorVec4(ImGuiCol_TitleBgActive));
        ImGui::PushStyleColor(ImGuiCol_TitleBgCollapsed, ImGui::GetStyleColorVec4(ImGuiCol_TitleBgActive));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));

        ImGui::Begin("AetherEditor", &m_Open, window_flags);
        ImGui::PopStyleColor(2);
        ImGui::PopStyleVar();

        DrawMainWindowMenuBar();

        if (ImGui::IsWindowHovered() && ImGui::IsMouseDragging(ImGuiMouseButton_Left))
        {
            if (!m_IsMainWindowDragging)
            {
                int x, y;
                glfwGetWindowPos(m_OsWindow->GetHandle(), &x, &y); // 当前 OS 窗口位置
                double cursorX, cursorY;
                glfwGetCursorPos(m_OsWindow->GetHandle(), &cursorX, &cursorY);
                m_MainWindowDragMouseStartPos.x = (float)cursorX;
                m_MainWindowDragMouseStartPos.y = (float)cursorY;
                m_MainWindowDragMouseStartPos.x += x;
                m_MainWindowDragMouseStartPos.y += y;
                m_MainWindowDragWindowStartPos.x = (float)x;
                m_MainWindowDragWindowStartPos.y = (float)y;
                m_IsMainWindowDragging = true;
            }
        }

        if (m_IsMainWindowDragging)
        {
            if (glfwGetMouseButton(m_OsWindow->GetHandle(), GLFW_MOUSE_BUTTON_LEFT) != GLFW_PRESS)
            {
                m_IsMainWindowDragging = false;
            }
        }

        if (m_IsMainWindowDragging)
        {
            int x, y;
            glfwGetWindowPos(m_OsWindow->GetHandle(), &x, &y); // 当前 OS 窗口位置
            double cursorX, cursorY;
            glfwGetCursorPos(m_OsWindow->GetHandle(), &cursorX, &cursorY);
            ImVec2 mousePos;
            mousePos.x = (float)cursorX;
            mousePos.y = (float)cursorY;

            mousePos.x += x;
            mousePos.y += y;

            ImVec2 delta;
            delta.x = mousePos.x - m_MainWindowDragMouseStartPos.x;

            delta.y = mousePos.y - m_MainWindowDragMouseStartPos.y;

            ImVec2 newWindowPos;
            newWindowPos.x = m_MainWindowDragWindowStartPos.x + delta.x;
            newWindowPos.y = m_MainWindowDragWindowStartPos.y + delta.y;
            glfwSetWindowPos(m_OsWindow->GetHandle(), newWindowPos.x, newWindowPos.y);
        }
        // 给父窗口创建独立 DockSpace

        ImGuiID dockspace_id = ImGui::GetID("Docking");
        ImGui::DockSpace(dockspace_id, ImVec2(0, 0));
    }
    void DrawBegin()
    {
        DrawMainWindowBegin();
        if (!m_Open)
        {
            if (m_OnClose)
            {
                m_OnClose();
            }
        }
    }

    void DrawEnd()
    {
        DrawMainWindowEnd();
    }
    void SetOnClose(std::function<void()>&& onClose)
    {
        m_OnClose = std::move(onClose);
    }
    void SetOnMaximize(std::function<void()>&& onMaximize)
    {
        m_OnMaximize = std::move(onMaximize);
    }

public:
    std::function<void()> m_OnClose;
    std::function<void()> m_OnMaximize;
    bool m_IsMainWindowDragging = false;
    ImVec2 m_MainWindowDragMouseStartPos;
    ImVec2 m_MainWindowDragWindowStartPos;
    Window* m_OsWindow;
    bool m_Open=true;
};