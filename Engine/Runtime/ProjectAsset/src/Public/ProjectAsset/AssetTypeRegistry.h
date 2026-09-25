#pragma once

#include <ProjectAsset/Types.h>

#include <functional>
#include <unordered_map>

namespace Aether::ProjectAssets
{
struct AssetTypeDescriptor
{
    AssetTypeId id;
    FeatureId ownerFeature;
    std::uint32_t formatVersion = 1;
    std::function<Result<Json>(const std::filesystem::path&)> loadMetadata;
    std::function<Result<std::vector<ProjectAssetRef>>(const std::filesystem::path&)> enumerateDependencies;
};

class AssetTypeRegistry
{
public:
    Result<void> Register(AssetTypeDescriptor descriptor)
    {
        if (m_Frozen) return std::unexpected(Error{ErrorCode::Frozen, "asset type registry is frozen"});
        if (!IsStableIdentifier(descriptor.id) || !IsStableIdentifier(descriptor.ownerFeature) || descriptor.formatVersion == 0)
            return std::unexpected(Error{ErrorCode::InvalidArgument, "asset type and owner must be stable identifiers with a non-zero version"});
        if (m_Entries.contains(descriptor.id))
            return std::unexpected(Error{ErrorCode::DuplicateId, "asset type is already registered", descriptor.id});
        m_Entries.emplace(descriptor.id, std::move(descriptor));
        return {};
    }
    const AssetTypeDescriptor* Find(std::string_view id) const noexcept
    {
        const auto it = m_Entries.find(std::string(id));
        return it == m_Entries.end() ? nullptr : &it->second;
    }
    void Freeze() noexcept { m_Frozen = true; }
    bool Frozen() const noexcept { return m_Frozen; }
    const auto& Entries() const noexcept { return m_Entries; }
private:
    std::unordered_map<AssetTypeId, AssetTypeDescriptor> m_Entries;
    bool m_Frozen = false;
};
}
