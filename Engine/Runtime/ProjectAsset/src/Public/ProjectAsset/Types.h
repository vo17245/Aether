#pragma once

#include <Core/UUID.h>
#include <Core/Serialization.h>

#include <cstdint>
#include <compare>
#include <expected>
#include <filesystem>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace Aether::ProjectAssets
{
template<class Tag>
class UuidId
{
public:
    UuidId() = default;
    static UuidId Create() { return UuidId(UUID::Create().ToString()); }
    static std::optional<UuidId> Parse(std::string_view value)
    {
        const auto parsed = UUID::FromString(std::string(value));
        if (!parsed || parsed->ToString() != value)
            return std::nullopt;
        return UuidId(parsed->ToString());
    }
    const std::string& ToString() const noexcept { return m_Value; }
    bool IsValid() const noexcept { return !m_Value.empty(); }
    friend bool operator==(const UuidId&, const UuidId&) = default;
    friend auto operator<=>(const UuidId&, const UuidId&) = default;
private:
    explicit UuidId(std::string value) : m_Value(std::move(value)) {}
    std::string m_Value;
};

struct ProjectIdTag;
struct AssetIdTag;
using ProjectId = UuidId<ProjectIdTag>;
using AssetId = UuidId<AssetIdTag>;
using FeatureId = std::string;
using AssetTypeId = std::string;

struct ProjectAssetRef
{
    ProjectId project;
    AssetId asset;
    friend bool operator==(const ProjectAssetRef&, const ProjectAssetRef&) = default;
};

struct ImportSourceFile
{
    std::string logicalName;
    std::filesystem::path originalPath;
};

struct ImportProvenance
{
    std::string importerId;
    std::uint32_t importerVersion = 0;
    Json settings = Json::object();
    std::vector<ImportSourceFile> sourceFiles;
};

struct AssetFileRecord
{
    std::string path;
    std::uint64_t byteSize = 0;
};

struct AssetRecord
{
    AssetId id;
    AssetTypeId type;
    FeatureId ownerFeature;
    std::uint32_t formatVersion = 1;
    std::uint64_t revision = 1;
    std::string displayPath;
    std::string artifactRoot;
    std::string entryPoint;
    std::vector<ProjectAssetRef> dependencies;
    std::vector<AssetFileRecord> files;
    ImportProvenance provenance;
};

enum class ErrorCode
{
    InvalidArgument, InvalidFormat, UnsupportedVersion, DuplicateId, MissingAsset,
    WrongProject, WrongType, UnsafePath, MissingFile, CorruptContent, DependencyCycle,
    DependencyMissing, LimitExceeded, Frozen, IoError, Conflict
};

struct Error
{
    ErrorCode code = ErrorCode::InvalidFormat;
    std::string message;
    std::string field;
};

template<class T>
using Result = std::expected<T, Error>;

struct Limits
{
    std::uint64_t maxManifestBytes = 16ull * 1024 * 1024;
    std::uint32_t maxRecords = 100000;
    std::uint32_t maxDependenciesPerAsset = 4096;
    std::uint64_t maxFilesPerRevision = 100000;
    std::uint64_t maxSingleFileBytes = 1ull * 1024 * 1024 * 1024;
    std::uint64_t maxTotalFileBytes = 4ull * 1024 * 1024 * 1024;
};

bool IsStableIdentifier(std::string_view value) noexcept;
}
