#pragma once
#include <vector>
#include <ImGui/ImGui.h>
#include <Project/ContentTree.h>
#include <AetherEditor/Panel/Panel.h>
#include "AssetTypeSearchPanel.h"
#include "CreateFolderPanel.h"
#include "Asset/CreateTexturePanel.h"
#include <AetherEditor/Global/GlobalProject.h>
#include <AetherEditor/UIComponent/Notify.h>
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
        if (ImGui::Button(".."))
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
                const char* name = child->GetName().data();
                if (child->GetName().size() == 0)
                {
                    name = "<Empty Name Folder>";
                }
                if (ImGui::Selectable(name))
                {
                    OnFolderSelectedEventHandler.Broadcast(static_cast<Aether::Project::Folder*>(child.get()));
                }
            }
            else if(child->GetType() == Aether::Project::ContentTreeNodeType::Asset)
            {
                const char* name = child->GetName().data();
                if (child->GetName().size() == 0)
                {
                    name = "<Empty Name Asset>";
                }
                if(ImGui::Selectable(name))
                {
                    OnAssetSelectedEventHandler.Broadcast(static_cast<Aether::Project::AssetContentNode*>(child.get()));
                }
            }
            else
            {
                ImGui::Text("<Unknown Node Type>");
            }
        }
    }
    void SetFolder(Aether::Project::Folder* folder)
    {
        m_Folder = folder;
        Aether::Project::ContentTreeNode* node = folder;
        std::vector<std::string> nodeNames;
        while (node)
        {
            nodeNames.push_back(std::string(node->GetName()));
            node = node->GetParent();
        }
        m_CurrentPath.clear();
        for (auto it = nodeNames.rbegin(); it != nodeNames.rend(); ++it)
        {
            m_CurrentPath += "/" + *it;
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
    Delegate<void(Aether::Project::AssetContentNode* assetNode)> OnAssetSelectedEventHandler;
};
class ContentBrowser : public Panel
{
public:
    ContentBrowser() : Panel("Content Browser")
    {
        m_FolderView.SetFolder(m_CurrentFolder);
        // 创建asset/folder时的回调
        m_AssetTypeSearchPanel->AssetTypeSelectedEventHandler +=
            [this](const std::string& type) { OnAssetTypeSelected(type); };
        // 创建文件夹的回调
        m_CreateFolderPanel->OnCreateFolderEventHandler += [this](const CreateFolderParams& params) {
            if (m_CurrentFolder)
            {
                auto folder = CreateScope<Aether::Project::Folder>(params.FolderName.c_str());
                folder->SetParent(m_CurrentFolder);
                m_CurrentFolder->AddChild(std::move(folder));
            }
        };
        m_FolderView.OnFolderSelectedEventHandler += [this](Aether::Project::Folder* folder) { SetFolder(folder); };
        m_CreateTexturePanel->TextureCreateEventHandler += [](Aether::Project::ImportTextureParams& params) {
            auto* project = GlobalProject::GetProject();
            if (!project)
            {
                Notify::Error("No project opened!");
                return;
            }
            Aether::Project::ImportTexture(*project, params);
        };
        m_FolderView.OnAssetSelectedEventHandler+= [this](Aether::Project::AssetContentNode* assetNode) {
            OnAssetClicked(*assetNode);
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
        m_CreateTexturePanel->OnImGuiUpdate();
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
    void OnAssetClicked(const Project::AssetContentNode& asset)
    {
        Notify::Info("Asset Clicked: " + std::string(asset.GetName()));
    }
    
    void OnAssetTypeSelected(const std::string& type)
    {
        if (type == "Folder")
        {
            m_CreateFolderPanel->SetVisible(true);
            auto mousePos = ImGui::GetMousePos();
            m_CreateFolderPanel->SetPosition(mousePos.x, mousePos.y);
        }
        else if (type == "Texture")
        {
            m_CreateTexturePanel->SetVisible(true);
            auto mousePos = ImGui::GetMousePos();
            m_CreateTexturePanel->SetPosition(mousePos.x, mousePos.y);
            m_CreateTexturePanel->SetFolderAddress(m_CurrentFolder->GetAddress());
        }
    }
    Scope<CreateFolderPanel> m_CreateFolderPanel{CreateScope<CreateFolderPanel>()};
    Scope<CreateTexturePanel> m_CreateTexturePanel{CreateScope<CreateTexturePanel>()};
};
} // namespace AetherEditor::UI