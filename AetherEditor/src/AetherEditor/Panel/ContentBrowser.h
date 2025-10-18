#pragma once
#include <vector>
#include <ImGui/ImGui.h>
#include <Project/ContentTree.h>
class FolderView
{
public:
    void OnImGuiUpdate()
    {
    }
    void SetFolder(Aether::Project::Folder* folder)
    {
        m_Folder = folder;
    }

private:
    Aether::Project::Folder* m_Folder = nullptr;
};
class ContentBrowser
{
public:
    ContentBrowser()
    {
        m_RootFolderView.SetFolder(&m_RootFolder);
    }
    void OnImGuiUpdate()
    {
        ImGui::Begin("Content Browser");
        m_RootFolderView.OnImGuiUpdate();
        ImGui::End();

    }

private:
    FolderView m_RootFolderView;
    Aether::Project::Folder m_RootFolder;
};