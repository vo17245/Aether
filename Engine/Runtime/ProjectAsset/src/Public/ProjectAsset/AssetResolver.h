#pragma once

#include <ProjectAsset/AssetTypeRegistry.h>

#include <memory>
#include <unordered_map>

namespace Aether::ProjectAssets
{
struct CatalogData
{
    ProjectId projectId;
    std::uint64_t generation = 0;
    std::vector<AssetRecord> records;
};

struct ResolvedAsset
{
    ProjectAssetRef reference;
    AssetTypeId type;
    std::uint64_t revision = 0;
    std::filesystem::path entryPath;
};

class CatalogSnapshot
{
public:
    static Result<std::shared_ptr<const CatalogSnapshot>> Create(CatalogData data, const AssetTypeRegistry& types);
    ProjectId Project() const noexcept { return m_Project; }
    std::uint64_t Generation() const noexcept { return m_Generation; }
    const AssetRecord* Find(AssetId id) const noexcept;
    const std::vector<AssetRecord>& Records() const noexcept { return m_Records; }
private:
    CatalogSnapshot(ProjectId project, std::uint64_t generation, std::vector<AssetRecord> records);
    ProjectId m_Project;
    std::uint64_t m_Generation = 0;
    std::vector<AssetRecord> m_Records;
    std::unordered_map<std::string, std::size_t> m_ById;
};

Result<ResolvedAsset> ResolveAsset(const CatalogSnapshot& catalog, const ProjectAssetRef& reference,
                                   std::string_view expectedType, const std::filesystem::path& projectRoot,
                                   const AssetTypeRegistry& types, std::string_view field = {});
Result<void> ValidateReferenceClosure(const CatalogSnapshot& catalog, const ProjectAssetRef& reference,
                                      const std::filesystem::path& projectRoot,
                                      const AssetTypeRegistry& types, std::string_view field = {});
Result<Json> EncodeCatalog(const CatalogSnapshot& catalog, const Limits& limits = {});
Result<CatalogData> DecodeCatalog(const Json& json, const Limits& limits = {});
}
