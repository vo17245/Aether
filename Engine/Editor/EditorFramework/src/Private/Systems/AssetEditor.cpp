#include <EditorFramework/AssetEditor.h>

#include <ProjectAsset/Types.h>

#include <unordered_set>

namespace Aether::EditorFramework
{
std::expected<void, std::string> AssetEditorRegistry::Register(AssetEditorDescriptor descriptor,
    const GameFeatures::RuntimeRegistry& runtime)
{
    if (m_Frozen) return std::unexpected(std::string("asset editor registry is frozen"));
    if (!ProjectAssets::IsStableIdentifier(descriptor.ownerFeature)
        || !ProjectAssets::IsStableIdentifier(descriptor.editorId) || descriptor.supportedTypes.empty()
        || !descriptor.createDocument)
        return std::unexpected(std::string("asset editor requires stable IDs, supported types, and a document factory"));
    if (!runtime.FindFeature(descriptor.ownerFeature))
        return std::unexpected(std::string("asset editor owner Feature is not in Runtime registry"));
    if (m_Editors.contains(descriptor.editorId)) return std::unexpected(std::string("asset editor ID is already registered"));
    std::unordered_set<std::string> uniqueTypes;
    for (const auto& assetType : descriptor.supportedTypes)
    {
        if (!ProjectAssets::IsStableIdentifier(assetType) || !uniqueTypes.emplace(assetType).second)
            return std::unexpected(std::string("asset editor has an invalid or duplicate supported type"));
        const auto* runtimeType = runtime.AssetTypes().Find(assetType);
        if (!runtimeType || runtimeType->ownerFeature != descriptor.ownerFeature)
            return std::unexpected(std::string("asset editor type is missing or owned by another Runtime Feature"));
        if (descriptor.isDefault && m_DefaultByType.contains(assetType))
            return std::unexpected(std::string("a default asset editor is already registered for type: ") + assetType);
    }
    const auto editorId = descriptor.editorId;
    m_Editors.emplace(editorId, std::move(descriptor));
    const auto& registered = m_Editors.at(editorId);
    if (registered.isDefault)
        for (const auto& assetType : registered.supportedTypes) m_DefaultByType.emplace(assetType, editorId);
    return {};
}

const AssetEditorDescriptor* AssetEditorRegistry::FindDefault(std::string_view assetType) const noexcept
{
    const auto route = m_DefaultByType.find(std::string(assetType));
    if (route == m_DefaultByType.end()) return nullptr;
    const auto editor = m_Editors.find(route->second);
    return editor == m_Editors.end() ? nullptr : &editor->second;
}

const AssetEditorDescriptor* AssetEditorRegistry::Find(std::string_view editorId) const noexcept
{
    const auto it = m_Editors.find(std::string(editorId));
    return it == m_Editors.end() ? nullptr : &it->second;
}

std::expected<OpenAssetDocumentResult, std::string> OpenAssetDocument(World& editorWorld,
    const ProjectAssets::CatalogSnapshot& catalog, const AssetEditorRegistry& editors,
    ProjectAssets::ProjectAssetRef reference)
{
    if (editorWorld.IsDispatching()) return std::unexpected(std::string("asset documents can only open at a safe point"));
    if (!reference.project.IsValid() || reference.project != catalog.Project() || !reference.asset.IsValid())
        return std::unexpected(std::string("asset reference is invalid or belongs to another project"));
    const auto* record = catalog.Find(reference.asset);
    if (!record) return std::unexpected(std::string("asset is missing from the committed catalog"));
    for (const auto entity : editorWorld.Select<DocumentComponent, AssetDocumentComponent, HistoryComponent>())
    {
        const auto& document = editorWorld.GetComponent<DocumentComponent>(entity);
        const auto& asset = editorWorld.GetComponent<AssetDocumentComponent>(entity);
        if (document.kind == DocumentKind::Asset && asset.projectId == reference.project && asset.assetId == reference.asset)
            return OpenAssetDocumentResult{entity, document.id, true, asset.readOnly};
    }

    const auto* descriptor = editors.FindDefault(record->type);
    const bool editable = descriptor && descriptor->createDocument && descriptor->prepareSave;
    const auto entity = editorWorld.CreateEntity();
    const auto documentId = GameFeatures::DocumentId::Create();
    editorWorld.AddComponent<DocumentComponent>(entity, DocumentComponent{documentId, DocumentKind::Asset, false, {}});
    editorWorld.AddComponent<AssetDocumentComponent>(entity,
        AssetDocumentComponent{reference.project, reference.asset, record->type, record->revision, !editable});
    editorWorld.AddComponent<HistoryComponent>(entity);
    if (editable)
    {
        try
        {
            auto created = descriptor->createDocument(editorWorld, entity, *record);
            if (!created)
            {
                editorWorld.DestroyEntity(entity);
                return std::unexpected(created.error());
            }
        }
        catch (const std::exception& exception)
        {
            editorWorld.DestroyEntity(entity);
            return std::unexpected(std::string("asset document factory failed: ") + exception.what());
        }
        catch (...)
        {
            editorWorld.DestroyEntity(entity);
            return std::unexpected(std::string("asset document factory failed with an unknown error"));
        }
    }
    return OpenAssetDocumentResult{entity, documentId, false, !editable};
}

std::expected<AssetImportTask, std::string> PrepareAssetDocumentSave(World& editorWorld, EntityId documentEntity,
    const AssetEditorRegistry& editors, const GameFeatures::RuntimeRegistry& runtime,
    const ProjectAssets::CatalogSnapshot& catalog,
    std::uint64_t sessionGeneration, const std::filesystem::path& projectRoot, ProjectAssets::Limits limits)
{
    if (editorWorld.IsDispatching()) return std::unexpected(std::string("asset documents can only be saved at a safe point"));
    if (!editorWorld.IsValid(documentEntity) || !editorWorld.HasComponent<DocumentComponent>(documentEntity)
        || !editorWorld.HasComponent<AssetDocumentComponent>(documentEntity))
        return std::unexpected(std::string("target is not an open asset document"));
    const auto& document = editorWorld.GetComponent<DocumentComponent>(documentEntity);
    const auto& asset = editorWorld.GetComponent<AssetDocumentComponent>(documentEntity);
    if (document.kind != DocumentKind::Asset || asset.readOnly)
        return std::unexpected(std::string("asset document is read-only"));
    if (sessionGeneration == 0 || !asset.projectId.IsValid() || asset.projectId != catalog.Project())
        return std::unexpected(std::string("asset document belongs to another or stale project session"));
    const auto* record = catalog.Find(asset.assetId);
    if (!record || record->type != asset.assetType || record->revision != asset.baseRevision)
        return std::unexpected(std::string("asset document base revision is stale"));
    const auto* editor = editors.FindDefault(asset.assetType);
    if (!editor || !editor->prepareSave)
        return std::unexpected(std::string("asset type has no writable default editor"));
    const auto* runtimeType = runtime.AssetTypes().Find(asset.assetType);
    if (!runtimeType || runtimeType->ownerFeature != editor->ownerFeature)
        return std::unexpected(std::string("asset editor type differs from Runtime registration"));
    std::expected<AssetImportProduct, std::string> product = std::unexpected(std::string("asset editor save preparation failed"));
    try { product = editor->prepareSave(editorWorld, documentEntity); }
    catch (const std::exception& exception) { return std::unexpected(std::string("asset editor save callback failed: ") + exception.what()); }
    catch (...) { return std::unexpected(std::string("asset editor save callback failed with an unknown error")); }
    if (!product) return std::unexpected(product.error());
    if (product->settings.is_object() == false)
        return std::unexpected(std::string("asset editor save settings must be a JSON object"));

    AssetImportRequest request;
    request.projectId = asset.projectId;
    request.sessionGeneration = sessionGeneration;
    request.catalogGeneration = catalog.Generation();
    request.importerId = editor->editorId;
    request.settings = product->settings;
    request.allowNoInputs = true;
    request.displayPath = product->displayPath;
    request.existingAsset = asset.assetId;
    request.baseRevision = asset.baseRevision;
    auto capturedProduct = std::move(*product);
    AssetImporterDescriptor worker;
    worker.id = editor->editorId;
    worker.ownerFeature = editor->ownerFeature;
    worker.outputType = asset.assetType;
    worker.outputFormatVersion = runtimeType->formatVersion;
    worker.import = [capturedProduct = std::move(capturedProduct)](const std::vector<ImportInputData>&,
        const Json&) mutable -> std::expected<AssetImportProduct, std::string> {
        return std::move(capturedProduct);
    };
    return StartAssetImport(std::move(request), std::move(worker), projectRoot, limits);
}

ProjectAssets::Result<std::shared_ptr<const ProjectAssets::CatalogSnapshot>> CommitAssetDocumentSave(
    World& editorWorld, EntityId documentEntity, PreparedAssetImport prepared,
    const AssetImportCommitContext& context, const ProjectAssets::CatalogSnapshot& currentCatalog,
    const GameFeatures::RuntimeRegistry& runtime, const std::filesystem::path& projectRoot,
    Storage::AtomicFileOperations* fileOperations, ProjectAssets::Limits limits)
{
    using namespace ProjectAssets;
    if (editorWorld.IsDispatching()) return std::unexpected(Error{ErrorCode::Frozen, "asset documents can only save at a safe point"});
    if (!editorWorld.IsValid(documentEntity) || !editorWorld.HasComponent<DocumentComponent>(documentEntity)
        || !editorWorld.HasComponent<AssetDocumentComponent>(documentEntity))
        return std::unexpected(Error{ErrorCode::Conflict, "asset document was closed before its save completed"});
    const auto& document = editorWorld.GetComponent<DocumentComponent>(documentEntity);
    const auto& asset = editorWorld.GetComponent<AssetDocumentComponent>(documentEntity);
    if (document.kind != DocumentKind::Asset || asset.readOnly || prepared.request.existingAsset != asset.assetId
        || prepared.request.baseRevision != asset.baseRevision || prepared.request.projectId != asset.projectId)
        return std::unexpected(Error{ErrorCode::Conflict, "save result targets a stale or read-only asset document"});
    if (context.currentSessionGeneration != prepared.request.sessionGeneration || context.currentProjectId != asset.projectId)
        return std::unexpected(Error{ErrorCode::Conflict, "save result belongs to a stale project session"});
    auto committed = CommitAssetImport(std::move(prepared), context, currentCatalog, runtime,
        projectRoot, fileOperations, limits);
    if (!committed) return std::unexpected(committed.error());
    const auto* updated = (*committed)->Find(asset.assetId);
    if (!updated) return std::unexpected(Error{ErrorCode::Conflict, "saved asset is absent from committed catalog"});
    editorWorld.GetComponent<AssetDocumentComponent>(documentEntity).baseRevision = updated->revision;
    editorWorld.GetComponent<DocumentComponent>(documentEntity).dirty = false;
    MarkDocumentSaved(editorWorld, documentEntity);
    return *committed;
}
}
