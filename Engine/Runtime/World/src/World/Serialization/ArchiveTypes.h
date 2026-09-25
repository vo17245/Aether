#pragma once

#include <Core/Serialization.h>
#include <entt/entt.hpp>

#include <cstdint>
#include <expected>
#include <filesystem>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace Aether { class World; }

namespace Aether::Serialization
{
// Entities carrying this component belong to a runtime service/preview and are
// excluded as a whole from persistent World archives.
struct ExcludeFromArchiveComponent { std::uint8_t marker = 0; };

enum class ArchiveErrorCode
{
    InvalidArgument, InvalidFormat, UnsupportedVersion, UnknownComponent, DuplicateRegistration,
    MissingAsset, UnsafePath, AlreadyExists, IoError, ResourceLimitExceeded, AllocationFailure, CodecFailure
};

struct ArchiveError
{
    ArchiveErrorCode code = ArchiveErrorCode::InvalidFormat;
    std::string message;
    std::filesystem::path path;
    std::string entityId;
    std::string componentType;
    std::string jsonPointer;
};

template<class T>
using Result = std::expected<T, ArchiveError>;

struct ArchiveLimits
{
    std::uint64_t maxManifestBytes = 16ull * 1024 * 1024;
    std::uint32_t maxJsonDepth = 64;
    std::uint64_t maxEntities = 100000;
    std::uint32_t maxComponentsPerEntity = 64;
    std::uint64_t maxComponents = 1000000;
    std::uint64_t maxAssetFiles = 100000;
    std::uint64_t maxSingleAssetBytes = 1ull * 1024 * 1024 * 1024;
    std::uint64_t maxTotalAssetBytes = 4ull * 1024 * 1024 * 1024;
};

struct AssetRef
{
    std::string path;
    std::string kind;
};

struct AssetEntry : AssetRef
{
    std::uint64_t byteSize = 0;
};

class AssetCatalog
{
public:
    const std::vector<AssetEntry>& Entries() const noexcept { return m_Entries; }
    const AssetEntry* Find(std::string_view path) const noexcept;
private:
    friend class SaveContext;
    friend Result<struct LoadedWorldDirectory> LoadWorldDirectory(const std::filesystem::path&, const class ComponentCodecRegistry&, const struct DirectoryLoadOptions&);
    std::vector<AssetEntry> m_Entries;
    std::unordered_map<std::string, std::size_t> m_ByPath;
};

class SaveContext
{
public:
    SaveContext() = default;
    const std::filesystem::path& SourceRoot() const noexcept { return m_SourceRoot; }
    const std::filesystem::path& PackageRoot() const noexcept { return m_PackageRoot; }
    Result<Json> EncodeEntity(entt::entity entity) const;
    Result<AssetRef> CopyFile(const std::filesystem::path& source, std::string kind);
    Result<AssetRef> WriteAsset(std::string kind, std::string extension,
                                const std::function<Result<void>(const std::filesystem::path&)>& writer);
    std::string CreateAssetGroup();
    Result<AssetRef> CopyGroupFile(std::string_view group, std::string_view relativePath,
                                  const std::filesystem::path& source, std::string kind);
    // Internal hook used by the directory implementation after a codec has produced a file.
    Result<void> RegisterAssetFile(std::string path, std::string kind, const std::filesystem::path& file);
private:
    friend class ::Aether::World;
    friend Result<void> SaveWorldDirectory(const World&, const class ComponentCodecRegistry&, const Json&,
                                           const std::filesystem::path&, const struct DirectorySaveOptions&);
    std::filesystem::path m_SourceRoot, m_PackageRoot;
    ArchiveLimits m_Limits{};
    std::unordered_map<std::uint32_t, std::string> m_EntityIds;
    std::vector<AssetEntry> m_Assets;
    std::unordered_map<std::string, std::uint64_t> m_AssetByPath;
    std::uint64_t m_NextGenerated = 1, m_NextGroup = 1, m_AssetBytes = 0;
};

class LoadContext
{
public:
    LoadContext() = default;
    Result<entt::entity> ResolveEntity(std::string_view id) const;
    Result<std::optional<entt::entity>> ResolveEntity(const Json& id) const;
    Result<std::filesystem::path> ResolveAsset(const AssetRef& ref, std::string_view expectedKind) const;
    Result<std::filesystem::path> ResolvePackagedPath(std::string_view relativePath, std::string_view expectedKind) const;
    const std::filesystem::path& PackageRoot() const noexcept { return m_PackageRoot; }
    const std::filesystem::path& SourceRoot() const noexcept { return m_SourceRoot; }
private:
    friend class ::Aether::World;
    friend Result<struct LoadedWorldDirectory> LoadWorldDirectory(const std::filesystem::path&, const class ComponentCodecRegistry&, const struct DirectoryLoadOptions&);
    std::filesystem::path m_SourceRoot, m_PackageRoot;
    std::unordered_map<std::string, entt::entity> m_Entities;
    ArchiveLimits m_Limits{};
    std::shared_ptr<const AssetCatalog> m_Assets;
};

struct DirectorySaveOptions
{
    ArchiveLimits limits{};
    std::filesystem::path sourceRoot;
};
struct DirectoryLoadOptions
{
    ArchiveLimits limits{};
};
struct LoadedWorldDirectory
{
    LoadedWorldDirectory(std::unique_ptr<World> world, Json application, std::filesystem::path packageRoot,
                         std::shared_ptr<const AssetCatalog> assets);
    ~LoadedWorldDirectory();
    LoadedWorldDirectory(LoadedWorldDirectory&&) noexcept;
    LoadedWorldDirectory& operator=(LoadedWorldDirectory&&) noexcept;
    LoadedWorldDirectory(const LoadedWorldDirectory&) = delete;
    LoadedWorldDirectory& operator=(const LoadedWorldDirectory&) = delete;
    std::unique_ptr<World> world;
    Json application;
    std::filesystem::path packageRoot;
    std::shared_ptr<const AssetCatalog> assets;
};

} // namespace Aether::Serialization
