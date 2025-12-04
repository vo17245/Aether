#pragma once
#include "Page.h"
#include <AetherEditor/Panel/Panel.h>
#include <Project/Project.h>
#include <AetherEditor/UIComponent/Notify.h>
namespace AetherEditor::UI
{

class AssetPage : public Page
{
public:
    AssetPage() : Page("AssetPage")
    {
    }
    void OnImGuiUpdate() override
    {
        ImGui::Begin("AssetPage");
        if(ImGui::Button("Return to Main Page"))
        {
            OnPageNavigateEventHandler.Broadcast("MainPage");
        }
        for(auto& [address,entry]:m_OpenedPanels)
        {
            auto& [assetName,panel]=entry;
            if(ImGui::Button(assetName.c_str()))
            {
                m_ActivePanel=panel.get();
            }
        }
        if (m_ActivePanel)
        {
            m_ActivePanel->OnImGuiUpdate();
        }
        ImGui::End();
    }
    void SetAssetToShow(const Project::AssetContentNode& assetNode);

private:
    struct Entry
    {
        std::string assetName;
        Scope<Panel> panel;
    };
    std::unordered_map<std::string,Entry> m_OpenedPanels;
    Panel* m_ActivePanel = nullptr;

public:
    Delegate<void(const std::string& name)> OnPageNavigateEventHandler;
};
} // namespace AetherEditor::UI