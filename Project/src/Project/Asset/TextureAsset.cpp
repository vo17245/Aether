#include "TextureAsset.h"
#include <Filesystem/Filesystem.h>
namespace Aether::Project
{
std::optional<std::string> ImportTexture(Project& project, ImportTextureParams& params)
{
    // copy image to address
    {
        std::string destPath = project.GetContentDirPath() + params.Address;
        std::string destDir=Filesystem::Path(destPath).Parent().GetStr();
        if(!Filesystem::Exists(destDir))
        {
            auto res=Filesystem::CreateDirectories(destDir);
            if(!res)
            {
                return "failed to create directories for texture import";
            }
        }
        auto err = Filesystem::CopyFile(params.FilePath, destPath);
        if (err)
        {
            return "failed to copy texture file to project content directory:\n" + *err;
        }
    }
    // create texture asset
    auto textureAsset = CreateScope<TextureAsset>();
    textureAsset->SetAddress(params.Address);
    textureAsset->SetName(params.Name);
    // add to project asset list
    project.GetAssets().emplace_back(std::move(textureAsset));
    auto* asset = project.GetAssets().back().get();
    // search content tree to add asset node
    auto arr=U32String(params.Address).Split("/");
    ContentTreeNode* currentNode=project.GetContentTreeRoot();
    if(arr.size()<=3)
    {
        // do nothing,asset is directly under root
    }
    else
    {
        for(size_t i=2;i<arr.size()-1;++i)
        {
            if(currentNode->GetType()!=ContentTreeNodeType::Folder)
            {
                return "invalid content tree structure";
            }
            std::string folderName=arr[i].ToStdString();
            auto* folderNode=static_cast<Folder*>(currentNode);
            auto* childNode=folderNode->FindChildByName(folderName);
            if(childNode==nullptr)
            {
                return "cannot find folder "+folderName+" in content tree";
            }
            currentNode=childNode;
        }
    }
    // create asset content node and add to currentNode
    if(!(currentNode->GetType()==ContentTreeNodeType::Folder))
    {
        return "invalid content tree structure";
    }
    auto& folderNode=static_cast<Folder&>(*currentNode);
    auto assetNode=CreateScope<AssetContentNode>();
    assetNode->SetAsset(asset);
    assetNode->SetAssetAddress(params.Address);
    assetNode->SetName(params.Name);
    folderNode.AddChild(std::move(assetNode));
    return std::nullopt;
}
} // namespace Aether::Project