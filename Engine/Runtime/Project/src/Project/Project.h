#pragma once
#include <Core/Core.h>
#include "Asset/Asset.h"
#include "ContentTree.h"
namespace Aether::Project
{

class Project
{
public:
    Project()
    {
        m_ContentTreeRoot = CreateScope<Folder>();
    }
    const std::vector<Scope<Asset>>& GetAssets() const
    {
        return m_Assets;
    }
    std::vector<Scope<Asset>>& GetAssets()
    {
        return m_Assets;
    }
    const std::string& GetProjectFilePath() const
    {
        return m_ProjectFilePath;
    }
    const std::string& GetContentDirPath() const
    {
        return m_ContentDirPath;
    }
    void SetProjectFilePath(const std::string& path)
    {
        m_ProjectFilePath = path;
    }
    void SetContentDirPath(const std::string& path)
    {
        m_ContentDirPath = path;
    }
    ContentTreeNode* GetContentTreeRoot() const
    {
        return m_ContentTreeRoot.get();
    }
    /**
     * @brief save project to file
     * @return std::optional<std::string> Returns std::nullopt on success, or an error message on failure.
    */
    std::optional<std::string> Save();
    static std::expected<Scope<Project>, std::string> LoadFromFile(const std::string& filePath);
    inline const std::string& GetProjectName() const
    {
        return m_ProjectName;
    }
    inline void SetProjectName(const std::string& name)
    {
        m_ProjectName = name;
    }
    std::optional<ContentTreeNode*> GetContent(const std::string& address);
private:
    std::string m_ProjectName;
    std::string m_ProjectFilePath;
    std::string m_ContentDirPath;
    std::vector<Scope<Asset>> m_Assets;
    Scope<ContentTreeNode> m_ContentTreeRoot;
};
} // namespace Aether::Project