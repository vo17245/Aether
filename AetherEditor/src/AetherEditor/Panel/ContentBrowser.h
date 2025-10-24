#pragma once
#include <vector>
#include <ImGui/ImGui.h>
#include <Project/ContentTree.h>
#include "Panel.h"
#include "AssetTypeSearchPanel.h"
class FolderView
{
public:
    void OnImGuiUpdate()
    {
        if (!m_Folder)
        {
            return;
        }
        ImGui::Text("Folder: %s", m_Folder->GetName().c_str());
    }
    void SetFolder(Aether::Project::Folder* folder)
    {
        m_Folder = folder;
    }
    virtual void SetPosition(float x, float y)
    {
    }
    virtual void SetSize(float width, float height)
    {
    }

private:
    Aether::Project::Folder* m_Folder = nullptr;
};
class ContentBrowser : public Panel
{
public:
    ContentBrowser()
    {
        m_RootFolderView.SetFolder(&m_RootFolder);
    }
    void OnImGuiUpdate() override
    {
        if (!m_Visible)
            return;
        ImGui::Begin(m_Title.c_str());
        m_RootFolderView.OnImGuiUpdate();
        m_AssetTypeSearchPanel->OnImGuiUpdate();
        if(IsRightClicked())
        {
            m_AssetTypeSearchPanel->SetVisible(true);
            auto mousePos=ImGui::GetMousePos();
            m_AssetTypeSearchPanel->SetPosition(mousePos.x,mousePos.y);
        }
        ImGui::End();

    }
    bool IsRightClicked()
    {
        return ImGui::IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByPopup | ImGuiHoveredFlags_AllowWhenBlockedByActiveItem) &&
               ImGui::IsMouseClicked(ImGuiMouseButton_Right);
    }
    
private:
    FolderView m_RootFolderView;
    Aether::Project::Folder m_RootFolder;
    Scope<AssetTypeSearchPanel> m_AssetTypeSearchPanel{ CreateScope<AssetTypeSearchPanel>() };
};