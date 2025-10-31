#include "Project.h"
#include <Filesystem/Filesystem.h>
namespace Aether::Project
{
    static Json ContentTreeNodeToJson(const ContentTreeNode& node)
    {
        if(node.GetType()==ContentTreeNodeType::Asset)
        {
            const auto& assetNode=static_cast<const AssetContentNode&>(node);
            Json j;
            j["Type"]="Asset";
            j["AssetAddress"]=assetNode.GetAssetAddress();
            return j;
        }
        else if(node.GetType()==ContentTreeNodeType::Folder)
        {
            const auto& folderNode=static_cast<const Folder&>(node);
            Json j;
            j["Type"]="Folder";
            j["Name"]=folderNode.GetName();
            // serialize children
            Json contentsJson=Json::array();
            for(auto& child:folderNode.GetChildren())
            {
                contentsJson.push_back(ContentTreeNodeToJson(*child));
            }
            j["Contents"]=contentsJson;
            return j;
        }
        else
        {
            return Json::string_t("UnknownNodeType");
        }
    }
    static Scope<ContentTreeNode> ContentTreeNodeFromJson(const Json& j)
    {
        std::string type=j["Type"];
        if(type=="Asset")
        {
            auto assetNode=CreateScope<AssetContentNode>();
            assetNode->SetAssetAddress(j["AssetAddress"].get<std::string>());
            return assetNode;
        }
        else if(type=="Folder")
        {
            auto folderNode=CreateScope<Folder>();
            folderNode->SetName(j["Name"].get<std::string>());
            for(auto& childJson:j["Contents"])
            {
                auto childNode=ContentTreeNodeFromJson(childJson);
                folderNode->AddChild(std::move(childNode));
            }
            return folderNode;
        }
        else
        {
            return nullptr;
        }
    }
    std::optional<std::string> Project::Save()
    {
        Json j;
        j["ProjectName"]= m_ProjectName;
        j["ContentDirPath"]= m_ContentDirPath;
        if(m_ContentTreeRoot)
        {
            j["ContentTree"]= ContentTreeNodeToJson(*m_ContentTreeRoot);
        }
        auto data=j.dump(4);
        auto writeResult=Filesystem::WriteFile(m_ProjectFilePath, data);
        if(!writeResult)
        {
            return std::optional<std::string>("Failed to write project file: " + m_ProjectFilePath);
        }
        return std::nullopt;
    }
    std::expected<Scope<Project>, std::string> Project::LoadFromFile(const std::string& filePath)
    {
        auto dataOpt=Filesystem::ReadFileToString(filePath);
        if(!dataOpt)
        {
            return std::unexpected("failed to read project file: "+filePath);
        }
        auto j=Json::parse(*dataOpt);
        auto project=CreateScope<Project>();
        project->m_ProjectFilePath=filePath;
        project->m_ProjectName=j["ProjectName"].get<std::string>();
        project->m_ContentDirPath=j["ContentDirPath"].get<std::string>();
        if(j.contains("ContentTree"))
        {
            project->m_ContentTreeRoot=ContentTreeNodeFromJson(j["ContentTree"]);
        }
        return project;
    }
}