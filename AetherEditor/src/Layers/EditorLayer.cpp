#include "EditorLayer.h"
#include "Aether/ImGui.h"
#include "Aether/ImGui/imgui.h"
#include "Core/MainScene.h"
#include "Panels/SceneViewPanel.h"
#include <memory>

namespace Aether
{
    namespace Editor
    {
        EditorLayer::EditorLayer()
        {
            m_Panels.emplace_back(std::make_unique<SceneViewPanel>());
        }
        void EditorLayer::OnImGuiRender()
        {
            
            // 设置停靠空间
            static bool p_open = true;
            static bool opt_fullscreen_persistant = true;
            bool opt_fullscreen = opt_fullscreen_persistant;
            ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;
            ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
            if (opt_fullscreen)
            {
                ImGuiViewport* viewport = ImGui::GetMainViewport();
                ImGui::SetNextWindowPos(viewport->Pos);
                ImGui::SetNextWindowSize(viewport->Size);
                ImGui::SetNextWindowViewport(viewport->ID);
                ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
                ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
                window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
                window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
            }

            // 将窗口放在最前面
            if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
                window_flags |= ImGuiWindowFlags_NoBackground;

            // 开始主窗口
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
            ImGui::Begin("DockSpace Demo", &p_open, window_flags);
            ImGui::PopStyleVar();

            if (opt_fullscreen)
                ImGui::PopStyleVar(2);

            // DockSpace
            ImGuiIO& io = ImGui::GetIO();
            if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
            {
                ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
                ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
            }
            else
            {
                // 显示错误信息
                ImGui::Text("ERROR: Docking is not enabled! Please enable docking in ImGui config flags (io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;)!");
            }

            // 添加菜单栏
            if (ImGui::BeginMenuBar())
            {
                if (ImGui::BeginMenu("File"))
                {
                    if (ImGui::MenuItem("Exit"))
                        Application::Get().Close();
                    ImGui::EndMenu();
                }
                if(ImGui::BeginMenu("Scene"))
                {
                    if(ImGui::MenuItem("Add Entity"))
                    {
                        MainScene::GetInstance().GetScene().CreateEntity();
                    }
                    ImGui::EndMenu();
                }
                ImGui::EndMenuBar();
            }
            //渲染panel
            for (auto& panel : m_Panels)
            {
                panel->OnImGuiRender();
            }
            // 结束主窗口
            ImGui::End();
            
        }
        void EditorLayer::OnRender()
        {
            for (auto& panel : m_Panels)
            {
                panel->OnRender();
            }
        }
    }//namespace Editor
}//namespace Aether