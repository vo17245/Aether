#pragma once

#include <EditorFramework/AssetImport.h>
#include <EditorFramework/Commands.h>

namespace Aether::EditorFramework
{
struct AssetEditorDescriptor
{
    GameFeatures::FeatureId ownerFeature;
    std::vector<ProjectAssets::AssetTypeId> supportedTypes;
    std::string editorId;
    bool isDefault = false;
    std::function<std::expected<void, std::string>(World&, EntityId, const ProjectAssets::AssetRecord&)> createDocument;
    std::function<std::expected<AssetImportProduct, std::string>(const World&, EntityId)> prepareSave;
};

class AssetEditorRegistry
{
public:
    std::expected<void, std::string> Register(AssetEditorDescriptor descriptor,
        const GameFeatures::RuntimeRegistry& runtime);
    void Freeze() noexcept { m_Frozen = true; }
    const AssetEditorDescriptor* FindDefault(std::string_view assetType) const noexcept;
    const AssetEditorDescriptor* Find(std::string_view editorId) const noexcept;
private:
    bool m_Frozen = false;
    std::unordered_map<std::string, AssetEditorDescriptor> m_Editors;
    std::unordered_map<std::string, std::string> m_DefaultByType;
};

struct OpenAssetDocumentResult
{
    EntityId entity = entt::null;
    GameFeatures::DocumentId documentId;
    bool reused = false;
    bool readOnly = true;
};

std::expected<OpenAssetDocumentResult, std::string> OpenAssetDocument(World& editorWorld,
    const ProjectAssets::CatalogSnapshot& catalog, const AssetEditorRegistry& editors,
    ProjectAssets::ProjectAssetRef reference);

std::expected<AssetImportTask, std::string> PrepareAssetDocumentSave(World& editorWorld, EntityId documentEntity,
    const AssetEditorRegistry& editors, const GameFeatures::RuntimeRegistry& runtime,
    const ProjectAssets::CatalogSnapshot& catalog,
    std::uint64_t sessionGeneration, const std::filesystem::path& projectRoot,
    ProjectAssets::Limits limits = {});

ProjectAssets::Result<std::shared_ptr<const ProjectAssets::CatalogSnapshot>> CommitAssetDocumentSave(
    World& editorWorld, EntityId documentEntity, PreparedAssetImport prepared,
    const AssetImportCommitContext& context, const ProjectAssets::CatalogSnapshot& currentCatalog,
    const GameFeatures::RuntimeRegistry& runtime, const std::filesystem::path& projectRoot,
    Storage::AtomicFileOperations* fileOperations = nullptr, ProjectAssets::Limits limits = {});
}
