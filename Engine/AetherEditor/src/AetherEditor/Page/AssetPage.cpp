#include "AssetPage.h"
#include <AetherEditor/Panel/Asset/TextureViewPanel.h>
namespace AetherEditor::UI
{
    void AssetPage::SetAssetToShow(const Project::AssetContentNode& assetNode)
    {
        auto* asset = assetNode.GetAsset();
        if(asset==nullptr)
        {
            Notify::Error("Asset is null");
            return;
        }

        // Check if panel for this asset is already opened
        auto it = m_OpenedPanels.find(assetNode.GetAssetAddress());
        if(it != m_OpenedPanels.end())
        {
            m_ActivePanel = it->second.panel.get();
            return;
        }
        
        if(asset->GetType()==Project::AssetType::Texture)
        {
            auto texturePanel = CreateScope<TextureViewPanel>();
            auto err=texturePanel->SetImageAddress(assetNode.GetAssetAddress());
            if(err)
            {
                Notify::Error(std::format("Failed to open texture asset: {}",*err));
                return;
            }
            m_ActivePanel=texturePanel.get();
            Entry entry;
            entry.assetName=asset->GetName();
            entry.panel=std::move(texturePanel);
            m_OpenedPanels[assetNode.GetAssetAddress()]=std::move(entry);
        }
    }
}