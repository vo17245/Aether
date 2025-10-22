#pragma once
#include <Imgui/Core/imgui.h>
#include <functional>
#include <Window/Window.h>
#include "Panel.h"
using namespace Aether;
class MainWindow
{
public:
    void SetOsWindow(Window* window)
    {
        m_OsWindow = window;
    }
    void DrawMainWindowEnd()
    {
        ImGui::End();
    }
    void DrawViewMenuItems()
    {
        if (ImGui::MenuItem("NodeEditor"))
        {
            if (m_NodeEditorViewToggle)
                m_NodeEditorViewToggle();
        }
        if (ImGui::MenuItem("Material Editor"))
        {
            if (m_MaterialEditorViewToggle)
                m_MaterialEditorViewToggle();
        }
        for(auto& panel : m_ViewPanels)
        {
            if (ImGui::MenuItem(panel->GetTitle().c_str()))
            {
                panel->SetVisible(!panel->GetVisible());
            }
        }
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
                if (ImGui::MenuItem("MetricsWindow"))
                {
                    m_ImGuiMetricsWindowOpen = !m_ImGuiMetricsWindowOpen;
                }
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("View"))
            {
                DrawViewMenuItems();

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
                                        | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_MenuBar
                                        | ImGuiWindowFlags_NoTitleBar;
        ImGui::PushStyleColor(ImGuiCol_TitleBg, ImGui::GetStyleColorVec4(ImGuiCol_TitleBgActive));
        ImGui::PushStyleColor(ImGuiCol_TitleBgCollapsed, ImGui::GetStyleColorVec4(ImGuiCol_TitleBgActive));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));

        ImGui::Begin("AetherEditor", nullptr, window_flags);
        ImGui::PopStyleColor(2);
        ImGui::PopStyleVar();

        DrawMainWindowMenuBar();

        // 给父窗口创建独立 DockSpace

        ImGuiID dockspace_id = ImGui::GetID("Docking");
        ImGui::DockSpace(dockspace_id, ImVec2(0, 0));
    }
    void DrawBegin()
    {
        DrawMainWindowBegin();
        if (m_ImGuiMetricsWindowOpen)
        {
            ImGui::ShowMetricsWindow();
        }
        for(auto& panel : m_ViewPanels)
        {
            panel->OnImGuiUpdate();
        }
    }

    void DrawEnd()
    {
        DrawMainWindowEnd();
    }
    void SetNodeEditorViewToggle(std::function<void()>&& toggle)
    {
        m_NodeEditorViewToggle = std::move(toggle);
    }
    void SetMaterialEditorViewToggle(std::function<void()>&& toggle)
    {
        m_MaterialEditorViewToggle = std::move(toggle);
    }
    void PushViewPanel(Scope<Panel>&& panel)
    {
        m_ViewPanels.push_back(std::move(panel));
    }
    void PopViewPanel(Panel* panel)
    {
        m_ViewPanels.erase(std::remove_if(m_ViewPanels.begin(), m_ViewPanels.end(),
                                          [panel](const Scope<Panel>& p) { return p.get() == panel; }),
                           m_ViewPanels.end());
    }

private:
    Window* m_OsWindow;
    bool m_ImGuiMetricsWindowOpen = false;
    std::function<void()> m_NodeEditorViewToggle;
    std::function<void()> m_MaterialEditorViewToggle;
    std::vector<Scope<Panel>> m_ViewPanels;
};