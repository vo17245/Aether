#include "MainPage.h"
#include <AetherEditor/Global/GlobalProject.h>
#include <AetherEditor/UIComponent/Notify.h>
namespace AetherEditor::UI
{
    MainPage::MainPage():Page("MainPage")
    {
        m_ContentBrowser.SetVisible(true);
        GlobalProject::ProjectChangedEventHandler+=[this](Aether::Project::Project& project)
        {
            assert(project.GetContentTreeRoot()->GetType()==Aether::Project::ContentTreeNodeType::Folder);
            auto* root = static_cast<Aether::Project::Folder*>(project.GetContentTreeRoot());
            m_ContentBrowser.SetFolder(root);
        };
        m_ContentBrowser.OnAssetClickedEventHandler+=[this](const Project::AssetContentNode& asset)
        {
            OnAssetClickedEventHandler.Broadcast(asset);
        };
        m_ContentBrowser.OnPageNavigateEventHandler+=[this](const std::string& name)
        {
            OnPageNavigateEventHandler.Broadcast(name);
        };
        
    }
    void MainPage::OnImGuiUpdate() 
    {
        
        if (ImGui::BeginMenuBar())
        {
            if (ImGui::BeginMenu("File"))
            {
                if (ImGui::MenuItem("Open", "Ctrl+O")) { /* 打开文件逻辑 */ }
                if (ImGui::MenuItem("Save", "Ctrl+S")) 
                {
                    auto err=GlobalProject::GetProject()->Save();
                    if(err)
                    {
                        Notify::Error(std::format("Failed to Save Project: {}",*err));
                    }
                }
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
            
            if (ImGui::BeginMenu("View"))
            {

                ImGui::EndMenu();
            }

            ImGui::EndMenuBar();
        }
        ImGuiID dockspace_id = ImGui::GetID("MainPageDocking");
        ImGui::DockSpace(dockspace_id, ImVec2(0, 0));
        m_ContentBrowser.OnImGuiUpdate();
    }
}