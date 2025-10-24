#pragma once
#include <Core/Core.h>
#include "Asset/Asset.h"
namespace Aether::Project
{
    class Project
    {
    public:
        
        const std::vector<Scope<Asset>>& GetAssets() const
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
    private:
        std::string m_ProjectFilePath;
        std::string m_ContentDirPath;
        std::vector<Scope<Asset>> m_Assets;
    };
}