#include "Project.h"
#include <Filesystem/Filesystem.h>
#include "Asset/TextureAsset.h"
namespace Aether::Project
{
static Json ContentTreeNodeToJson(const ContentTreeNode& node)
{
    if (node.GetType() == ContentTreeNodeType::Asset)
    {
        const auto& assetNode = static_cast<const AssetContentNode&>(node);
        Json j;
        j["Name"] = assetNode.GetName();
        j["Type"] = "Asset";
        j["AssetAddress"] = assetNode.GetAssetAddress();

        return j;
    }
    else if (node.GetType() == ContentTreeNodeType::Folder)
    {
        const auto& folderNode = static_cast<const Folder&>(node);
        Json j;
        j["Type"] = "Folder";
        j["Name"] = folderNode.GetName();
        // serialize children
        Json contentsJson = Json::array();
        for (auto& child : folderNode.GetChildren())
        {
            contentsJson.push_back(ContentTreeNodeToJson(*child));
        }
        j["Contents"] = contentsJson;
        return j;
    }
    else
    {
        return Json::string_t("UnknownNodeType");
    }
}
static Scope<ContentTreeNode> ContentTreeNodeFromJson(const Json& j)
{
    std::string type = j["Type"];
    if (type == "Asset")
    {
        auto assetNode = CreateScope<AssetContentNode>();
        assetNode->SetAssetAddress(j["AssetAddress"].get<std::string>());
        assetNode->SetName(j["Name"].get<std::string>());
        return assetNode;
    }
    else if (type == "Folder")
    {
        auto folderNode = CreateScope<Folder>();
        folderNode->SetName(j["Name"].get<std::string>());
        for (auto& childJson : j["Contents"])
        {
            auto childNode = ContentTreeNodeFromJson(childJson);
            childNode->SetParent(folderNode.get());
            folderNode->AddChild(std::move(childNode));
        }
        return folderNode;
    }
    else
    {
        return nullptr;
    }
}
static Json AssetToJson(const Asset& asset)
{
    // base asset info
    Json j;
    j["ID"] = asset.GetID().ToString();
    j["Name"] = asset.GetName();
    j["Address"] = asset.GetAddress();
    j["Type"] = AssetTypeToString(asset.GetType());
    // specific asset type info
    switch (asset.GetType())
    {
    case AssetType::Texture: {
        const auto& textureAsset = static_cast<const TextureAsset&>(asset);
        j["sRGB"] = textureAsset.sRGB;
    }
    break;
    default:
        LogE("AssetToJson: Unknown asset type enum value: {}", static_cast<int>(asset.GetType()));
        break;
    }
    return j;
}
static Scope<Asset> AssetFromJson(const Json& j)
{
    AssetType type = AssetTypeFromString(j["Type"].get<std::string>());
    Scope<Asset> asset;
    switch (type)
    {
    case AssetType::Texture: {
        auto textureAsset = CreateScope<TextureAsset>();
        textureAsset->sRGB = j["sRGB"].get<bool>();
        asset = std::move(textureAsset);
    }
    break;
    default:
        LogE("AssetFromJson: Unknown asset type string: {}", j["Type"].get<std::string>());
        return nullptr;
    }
    asset->SetName(j["Name"].get<std::string>());
    asset->SetAddress(j["Address"].get<std::string>());
    // Note: ID is not set here as it is usually generated on asset creation
    return asset;
}
static Json AssetArrayToJson(const std::vector<Scope<Asset>>& assets)
{
    Json j = Json::array();
    for (const auto& asset : assets)
    {
        j.push_back(AssetToJson(*asset));
    }
    return j;
}
static std::vector<Scope<Asset>> AssetArrayFromJson(const Json& j)
{
    std::vector<Scope<Asset>> assets;
    for (const auto& assetJson : j)
    {
        auto asset = AssetFromJson(assetJson);
        if (asset)
        {
            assets.push_back(std::move(asset));
        }
    }
    return assets;
}
std::optional<std::string> Project::Save()
{
    Json j;
    j["ProjectName"] = m_ProjectName;
    j["ContentDirPath"] = m_ContentDirPath;
    if (m_ContentTreeRoot)
    {
        j["ContentTree"] = ContentTreeNodeToJson(*m_ContentTreeRoot);
    }
    j["Assets"] = AssetArrayToJson(m_Assets);
    auto data = j.dump(4);
    auto writeResult = Filesystem::WriteFile(m_ProjectFilePath, data);
    if (!writeResult)
    {
        return std::optional<std::string>("Failed to write project file: " + m_ProjectFilePath);
    }
    return std::nullopt;
}
std::expected<Scope<Project>, std::string> Project::LoadFromFile(const std::string& filePath)
{
    auto dataOpt = Filesystem::ReadFileToString(filePath);
    if (!dataOpt)
    {
        return std::unexpected("failed to read project file: " + filePath);
    }
    auto j = Json::parse(*dataOpt);
    auto project = CreateScope<Project>();
    project->m_ProjectFilePath = filePath;
    project->m_ProjectName = j["ProjectName"].get<std::string>();
    project->m_ContentDirPath = j["ContentDirPath"].get<std::string>();
    if (j.contains("ContentTree"))
    {
        project->m_ContentTreeRoot = ContentTreeNodeFromJson(j["ContentTree"]);
    }
    project->m_Assets = AssetArrayFromJson(j["Assets"]);
    return project;
}
std::optional<ContentTreeNode*> Project::GetContent(const std::string& address)
{
    U32String addressU32=U32String(address);
    auto arr=addressU32.Split("/");
    ContentTreeNode* currentNode=m_ContentTreeRoot.get();
    for(const auto& part:arr)
    {
        auto* next=currentNode->FindChildByName(part.ToStdString());
        if(!next)
        {
            return std::nullopt;
        }
        currentNode=next;
    }
    return currentNode;
}
} // namespace Aether::Project