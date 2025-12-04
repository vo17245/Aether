#pragma once
#include <string>
#include <unordered_map>
#include <vector>
#include "Filesystem/Def.h"
#include "Filesystem/Filesystem.h"
#include "AssetInfo.h"
#include <string>
#include "AssetBundleInfo.h"
#include "Filesystem/FilesystemApi.h"
namespace Aether::Resource
{
class Finder
{
public:
    struct FindResult
    {
        Scope<AssetInfo> info;
        std::string path;
    };
    void AddBundleDir(const std::string_view dir)
    {
        m_BundleDirs.emplace_back(dir);
    }
    std::expected<FindResult, std::string> Find(const std::string_view address)
    {
        for (const auto& dir : m_BundleDirs)
        {
            std::string path = dir + "/" + std::string(address);
            // Check if the file exists
            if (!(Filesystem::Exists(path) && Filesystem::IsFile(path)))
            {
                continue;
            }
            // get bundle info
            std::string bundleInfoPath = dir + "/bundle.json";
            if (!(Filesystem::Exists(bundleInfoPath) && Filesystem::IsFile(bundleInfoPath)))
            {
                return std::unexpected<std::string>("bundle.json not found");
            }
            // try get from cache
            auto iter = m_AssetBundles.find(bundleInfoPath);
            if (iter == m_AssetBundles.end())
            {
                // load from file
                Filesystem::File file(bundleInfoPath,Filesystem::Action::Read);
                if (!file.IsOpened())
                {
                    return std::unexpected<std::string>(std::format("failed to open bundle info: {}", bundleInfoPath));
                }
                std::string fileContent(file.GetSize(),0);
                bool readRes=file.Read(std::span<uint8_t>((uint8_t*)fileContent.data(),fileContent.size()));
                if(!readRes)
                {
                    return std::unexpected<std::string>(std::format("failed to read bundle info: {}", bundleInfoPath));
                }
                auto bundleInfo = FromJson<AssetBundleInfo>(Json::parse(fileContent));
                if (!bundleInfo)
                {
                    return std::unexpected<std::string>(std::format("failed to parse bundle info: {}", bundleInfo.error()));
                }
                m_AssetBundles[bundleInfoPath] = std::move(bundleInfo.value());
            }
            auto& bundleInfo = m_AssetBundles[bundleInfoPath];
            // find asset info
            auto iter2 = bundleInfo.assets.find(std::string(address));
            FindResult result;
            result.path = std::move(path);
            if (iter2 != bundleInfo.assets.end())
            {
                result.info = Scope<AssetInfo>(iter2->second->Clone());
            }
            return result;
        }
        return std::unexpected<std::string>("Asset not found");
    }

private:
    std::vector<std::string> m_BundleDirs;
    std::unordered_map<std::string, AssetBundleInfo> m_AssetBundles;
};
} // namespace Aether::Resource