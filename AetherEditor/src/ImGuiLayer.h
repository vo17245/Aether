#pragma once
#include "Window/Layer.h"
#include "Window/Window.h"
#include "Window/WindowContext.h"
#include "Render/Mesh/VkMesh.h"
#include "Render/Mesh/Geometry.h"
#include "Render/Shader/Compiler.h"
#include "Render/Utils.h"
#include <print>
#include "Entry/Application.h"
#include <Render/RenderGraph/RenderGraph.h>
#include <IO/Image.h>
#include <Debug/Log.h>
#include "Material.h"
using namespace Aether;
#include "Notify.h"
#include "Panel/MaterialPanel.h"
#include "FileWatcher.h"
#include "Panel/ScenePanel.h"
#include <UI/Render/LoadTextureToLinear.h>
#include "Utils/LoadTexture.h"
#include "Panel/TerminalPanel.h"
class ImGuiLayer : public Layer
{
public:
    ~ImGuiLayer()
    {
    }

    virtual void OnAttach(Window* window) override
    {
        m_Window = window;
        // Notify::Error("This is an error message", 10.0f);
        // Notify::Info("This is an info message", 10.0f);
        // Notify::Warning("This is an warning message", 10.0f);
        m_MaterialPanel.Open();
        m_DummyScene = Utils::LoadSrgbTexture("Assets/bundle/Images/logo.png").value();
        m_ScenePanel.SetTexture(m_DummyScene);
    }
    virtual void OnUpdate(float sec) override
    {
        Notify::Update(sec);
        m_MaterialPanel.OnUpdate(sec);
    }
    virtual void RegisterRenderPasses(RenderGraph::RenderGraph& renderGraph) override
    {
    }
    virtual bool NeedRebuildRenderGraph() override
    {
        return m_NeedRebuild;
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

            ImGui::EndMenuBar();
        }
    }
    void DrawMainWindowBegin()
    {
        Vec2i size = m_Window->GetSize();
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
                glfwGetWindowPos(m_Window->GetHandle(), &x, &y); // 当前 OS 窗口位置
                double cursorX, cursorY;
                glfwGetCursorPos(m_Window->GetHandle(), &cursorX, &cursorY);
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
            if (glfwGetMouseButton(m_Window->GetHandle(), GLFW_MOUSE_BUTTON_LEFT) != GLFW_PRESS)
            {
                m_IsMainWindowDragging = false;
            }
        }

        if (m_IsMainWindowDragging)
        {
            int x, y;
            glfwGetWindowPos(m_Window->GetHandle(), &x, &y); // 当前 OS 窗口位置
            double cursorX, cursorY;
            glfwGetCursorPos(m_Window->GetHandle(), &cursorX, &cursorY);
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
            glfwSetWindowPos(m_Window->GetHandle(), newWindowPos.x, newWindowPos.y);
        }
        // 给父窗口创建独立 DockSpace

        ImGuiID dockspace_id = ImGui::GetID("Docking");
        ImGui::DockSpace(dockspace_id, ImVec2(0, 0));
    }
    virtual void OnImGuiUpdate() override
    {
        Vec2i size = m_Window->GetSize();

        DrawMainWindowBegin();
        if (!m_Open)
        {
            if (m_OnClose)
            {
                m_OnClose();
            }
        }
        m_MaterialPanel.Draw();
        m_ScenePanel.Draw();
        m_TerminalPanel.Draw();
        Notify::Draw();
        DrawMainWindowEnd();
    }
    void SetOnClose(std::function<void()>&& onClose)
    {
        m_OnClose = std::move(onClose);
    }

private:
    bool m_NeedRebuild;
    Window* m_Window = nullptr;
    MaterialPanel m_MaterialPanel;
    float m_Float;
    bool m_Open = true;
    std::function<void()> m_OnClose;

private:
    bool m_IsMainWindowDragging = false;
    ImVec2 m_MainWindowDragMouseStartPos;
    ImVec2 m_MainWindowDragWindowStartPos;

private:
    ScenePanel m_ScenePanel;
    DeviceTexture m_DummyScene;
    TerminalPanel m_TerminalPanel;
};