#pragma once
#include <vector>
#include <ImGui/ImGui.h>
#include <Project/ContentTree.h>
#include <AetherEditor/Panel/Panel.h>
#include "AssetTypeSearchPanel.h"
#include "CreateFolderPanel.h"
namespace AetherEditor::UI
{

class FolderView
{
public:
    void OnImGuiUpdate()
    {
        if (!m_Folder)
        {
            return;
        }
        ImGui::Text("Current Path: %s", m_CurrentPath.c_str());
        ImGui::Separator();
        if(ImGui::Button( ".."))
        {
            if (m_Folder->GetParent())
            {
                OnFolderSelectedEventHandler.Broadcast(static_cast<Aether::Project::Folder*>(m_Folder->GetParent()));
            }
        }
        ImGui::Separator();
        for (auto& child : m_Folder->GetChildren())
        {
            if (child->GetType() == Aether::Project::ContentTreeNodeType::Folder)
            {
                if (ImGui::Selectable(child->GetName().data()))
                {
                    OnFolderSelectedEventHandler.Broadcast(static_cast<Aether::Project::Folder*>(child.get()));
                }
            }
        }
    }
    void SetFolder(Aether::Project::Folder* folder)
    {
        m_Folder = folder;
        Aether::Project::ContentTreeNode* node = folder;
        while (node)
        {
            m_CurrentPath = "/" + std::string(node->GetName()) + m_CurrentPath;
            node = node->GetParent();
        }
    }
    virtual void SetPosition(float x, float y)
    {
    }
    virtual void SetSize(float width, float height)
    {
    }

private:
    Aether::Project::Folder* m_Folder = nullptr;
    std::string m_CurrentPath;
public:
    Delegate<void(Aether::Project::Folder* folder)> OnFolderSelectedEventHandler;
};
class ContentBrowser : public Panel
{
public:
    ContentBrowser() : Panel("Content Browser")
    {
        m_FolderView.SetFolder(m_CurrentFolder);
        m_AssetTypeSearchPanel->AssetTypeSelectedEventHandler +=
            [this](const std::string& type) { OnAssetTypeSelected(type); };
        m_CreateFolderPanel->OnCreateFolderEventHandler += [this](const CreateFolderParams& params) {
            if (m_CurrentFolder)
            {
                auto folder = CreateScope<Aether::Project::Folder>(params.FolderName.c_str());
                m_CurrentFolder->AddChild(std::move(folder));
            }
        };
        m_FolderView.OnFolderSelectedEventHandler += [this](Aether::Project::Folder* folder) {
            SetFolder(folder);
        };
    }
    void OnImGuiUpdate() override
    {
        if (!m_Visible)
            return;
        ImGui::Begin(m_Title.c_str());
        m_FolderView.OnImGuiUpdate();
        m_AssetTypeSearchPanel->OnImGuiUpdate();
        if (IsRightClicked())
        {
            m_AssetTypeSearchPanel->SetVisible(true);
            auto mousePos = ImGui::GetMousePos();
            m_AssetTypeSearchPanel->SetPosition(mousePos.x, mousePos.y);
        }
        m_CreateFolderPanel->OnImGuiUpdate();
        ImGui::End();
    }
    bool IsRightClicked()
    {
        return ImGui::IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByPopup
                                      | ImGuiHoveredFlags_AllowWhenBlockedByActiveItem)
               && ImGui::IsMouseClicked(ImGuiMouseButton_Right);
    }
    void SetFolder(Aether::Project::Folder* folder)
    {
        m_CurrentFolder = folder;
        m_FolderView.SetFolder(m_CurrentFolder);
    }

private:
    FolderView m_FolderView;
    Aether::Project::Folder* m_CurrentFolder = nullptr;
    Scope<AssetTypeSearchPanel> m_AssetTypeSearchPanel{CreateScope<AssetTypeSearchPanel>()};

private:
    void OnAssetTypeSelected(const std::string& type)
    {
        if (type == "Folder")
        {
            m_CreateFolderPanel->SetVisible(true);
            auto mousePos = ImGui::GetMousePos();
            m_CreateFolderPanel->SetPosition(mousePos.x, mousePos.y);
        }
    }
    Scope<CreateFolderPanel> m_CreateFolderPanel{CreateScope<CreateFolderPanel>()};
};
} // namespace AetherEditor::UI