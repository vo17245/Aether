#pragma once

#include <EditorFramework/Storage/AtomicReplaceFile.h>
#include <GameFeature/RuntimeFeatureRegistrar.h>
#include <ProjectAsset/AssetResolver.h>

#include <atomic>
#include <functional>
#include <future>
#include <span>
#include <unordered_map>
#include <utility>

namespace Aether::EditorFramework
{
struct ImportInput
{
    std::string logicalName;
    std::filesystem::path sourcePath;
};

struct ImportInputData
{
    std::string logicalName;
    std::filesystem::path sourcePath;
    std::vector<std::byte> contents;
};

struct ImportOutputFile
{
    std::string path;
    std::vector<std::byte> contents;
};

struct AssetImportProduct
{
    std::string displayPath;
    std::string entryPoint;
    Json settings = Json::object();
    std::vector<ImportOutputFile> sources;
    std::vector<ImportOutputFile> artifacts;
    std::vector<ProjectAssets::ProjectAssetRef> dependencies;
};

struct AssetImporterDescriptor
{
    std::string id;
    GameFeatures::FeatureId ownerFeature;
    ProjectAssets::AssetTypeId outputType;
    std::uint32_t outputFormatVersion = 0;
    std::uint32_t version = 1;
    std::vector<std::string> extensions;
    std::function<bool(std::span<const std::byte>)> probe;
    std::function<std::expected<AssetImportProduct, std::string>(
        const std::vector<ImportInputData>&, const Json&)> import;
    // Checked before allocating ImportInputData::contents. Zero uses the project limit.
    std::uint64_t maxInputBytes = 0;
};

class AssetImporterRegistry
{
public:
    std::expected<void, std::string> Register(AssetImporterDescriptor descriptor,
        const GameFeatures::RuntimeRegistry& runtime);
    void Freeze() noexcept { m_Frozen = true; }
    bool Frozen() const noexcept { return m_Frozen; }
    const AssetImporterDescriptor* Find(std::string_view id) const noexcept;
    std::vector<std::string> Candidates(const std::filesystem::path& input,
        std::span<const std::byte> leadingBytes) const;
private:
    bool m_Frozen = false;
    std::unordered_map<std::string, AssetImporterDescriptor> m_Entries;
};

struct AssetImportRequest
{
    ProjectAssets::ProjectId projectId;
    std::uint64_t sessionGeneration = 0;
    std::uint64_t catalogGeneration = 0;
    std::string importerId;
    std::vector<ImportInput> inputs;
    Json settings = Json::object();
    bool allowNoInputs = false;
    std::string displayPath;
    std::optional<ProjectAssets::AssetId> existingAsset;
    std::uint64_t baseRevision = 0;
};

struct PreparedAssetImport
{
    AssetImportRequest request;
    ProjectAssets::AssetRecord record;
    std::filesystem::path stagingDirectory;
    std::shared_ptr<std::atomic_bool> cancelled;

    PreparedAssetImport() = default;
    PreparedAssetImport(AssetImportRequest requestValue, ProjectAssets::AssetRecord recordValue,
        std::filesystem::path stagingPath, std::shared_ptr<std::atomic_bool> cancellation)
        : request(std::move(requestValue)), record(std::move(recordValue)),
          stagingDirectory(std::move(stagingPath)), cancelled(std::move(cancellation)) {}
    PreparedAssetImport(const PreparedAssetImport&) = delete;
    PreparedAssetImport& operator=(const PreparedAssetImport&) = delete;
    PreparedAssetImport(PreparedAssetImport&& other) noexcept
        : request(std::move(other.request)), record(std::move(other.record)),
          stagingDirectory(std::exchange(other.stagingDirectory, {})), cancelled(std::move(other.cancelled)) {}
    PreparedAssetImport& operator=(PreparedAssetImport&& other) noexcept
    {
        if (this != &other)
        {
            Cleanup();
            request = std::move(other.request);
            record = std::move(other.record);
            stagingDirectory = std::exchange(other.stagingDirectory, {});
            cancelled = std::move(other.cancelled);
        }
        return *this;
    }
    ~PreparedAssetImport() { Cleanup(); }
private:
    void Cleanup() noexcept
    {
        if (!stagingDirectory.empty())
        {
            std::error_code ignored;
            std::filesystem::remove_all(stagingDirectory, ignored);
            stagingDirectory.clear();
        }
    }
};

struct AssetImportTask
{
    std::shared_ptr<std::atomic_bool> cancelled;
    std::future<std::expected<PreparedAssetImport, std::string>> result;
    void Cancel() noexcept { if (cancelled) cancelled->store(true, std::memory_order_relaxed); }
};

AssetImportTask StartAssetImport(AssetImportRequest request, AssetImporterDescriptor importer,
    std::filesystem::path projectRoot, ProjectAssets::Limits limits = {});

struct AssetImportCommitContext
{
    ProjectAssets::ProjectId currentProjectId;
    std::uint64_t currentSessionGeneration = 0;
    bool playLocked = false;
    bool writable = true;
};

struct AssetRenameRequest
{
    ProjectAssets::ProjectId projectId;
    std::uint64_t sessionGeneration = 0;
    std::uint64_t catalogGeneration = 0;
    ProjectAssets::AssetId assetId;
    std::string displayPath;
};

enum class AssetImportTaskState { Queued, Running, Committed, Rejected, Cancelled };
struct AssetImportRequestComponent
{
    AssetImportRequest request;
    bool cancelled = false;
};
struct AssetImportTaskComponent
{
    AssetImportTask task;
};
struct AssetImportResultComponent
{
    AssetImportTaskState state = AssetImportTaskState::Queued;
    std::string error;
    std::optional<ProjectAssets::ProjectAssetRef> asset;
    std::uint64_t revision = 0;
};

EntityId SubmitAssetImport(World& editorWorld, AssetImportRequest request);
void CancelAssetImport(World& editorWorld, EntityId requestEntity);
void CommitReadyImports(World& editorWorld);

ProjectAssets::Result<std::shared_ptr<const ProjectAssets::CatalogSnapshot>> CommitAssetImport(
    PreparedAssetImport prepared, const AssetImportCommitContext& context,
    const ProjectAssets::CatalogSnapshot& currentCatalog, const GameFeatures::RuntimeRegistry& runtime,
    const std::filesystem::path& projectRoot, Storage::AtomicFileOperations* fileOperations = nullptr,
    ProjectAssets::Limits limits = {});

ProjectAssets::Result<std::shared_ptr<const ProjectAssets::CatalogSnapshot>> CommitAssetRename(
    const AssetRenameRequest& request, const AssetImportCommitContext& context,
    const ProjectAssets::CatalogSnapshot& currentCatalog, const GameFeatures::RuntimeRegistry& runtime,
    const std::filesystem::path& projectRoot, Storage::AtomicFileOperations* fileOperations = nullptr,
    ProjectAssets::Limits limits = {});
}
