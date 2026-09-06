#pragma once
#include <Imgui/Core/imgui.h>
#include <functional>
#include <Window/Window.h>
using namespace Aether;
namespace AetherEditor::UI
{

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
                const Vec2i windowPosition = m_OsWindow->GetPosition();
                const Vec2f cursorPosition = m_OsWindow->GetCursorPosition();
                m_MainWindowDragMouseStartPos.x = cursorPosition.x() + windowPosition.x();
                m_MainWindowDragMouseStartPos.y = cursorPosition.y() + windowPosition.y();
                m_MainWindowDragWindowStartPos.x = static_cast<float>(windowPosition.x());
                m_MainWindowDragWindowStartPos.y = static_cast<float>(windowPosition.y());
                m_IsMainWindowDragging = true;
            }
        }

        if (m_IsMainWindowDragging)
        {
            if (!m_OsWindow->IsMouseButtonPressed(MouseButtonCode::Left))
            {
                m_IsMainWindowDragging = false;
            }
        }

        if (m_IsMainWindowDragging)
        {
            const Vec2i windowPosition = m_OsWindow->GetPosition();
            const Vec2f cursorPosition = m_OsWindow->GetCursorPosition();
            ImVec2 mousePos;
            mousePos.x = cursorPosition.x() + windowPosition.x();
            mousePos.y = cursorPosition.y() + windowPosition.y();

            ImVec2 delta;
            delta.x = mousePos.x - m_MainWindowDragMouseStartPos.x;

            delta.y = mousePos.y - m_MainWindowDragMouseStartPos.y;

            ImVec2 newWindowPos;
            newWindowPos.x = m_MainWindowDragWindowStartPos.x + delta.x;
            newWindowPos.y = m_MainWindowDragWindowStartPos.y + delta.y;
            m_OsWindow->SetPosition(static_cast<int>(newWindowPos.x), static_cast<int>(newWindowPos.y));
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
}