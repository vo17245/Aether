#include <EditorFramework/AssetEditor.h>

#include <cassert>
#include <fstream>

namespace
{
struct DraftValue
{
    std::uint64_t value = 1;
    std::string displayPath;
};

std::vector<std::byte> Bytes(std::string_view value)
{
    const auto data = std::as_bytes(std::span(value.data(), value.size()));
    return {data.begin(), data.end()};
}

class FailReplace final : public Aether::EditorFramework::Storage::AtomicFileOperations
{
public:
    Aether::EditorFramework::Storage::FileResult Write(const std::filesystem::path& path,
        std::span<const std::byte> contents) override
    {
        std::ofstream stream(path, std::ios::binary);
        stream.write(reinterpret_cast<const char*>(contents.data()), static_cast<std::streamsize>(contents.size()));
        return stream ? Aether::EditorFramework::Storage::FileResult{} : std::unexpected("write failed");
    }
    Aether::EditorFramework::Storage::FileResult Flush(const std::filesystem::path&) override { return {}; }
    Aether::EditorFramework::Storage::FileResult Replace(const std::filesystem::path&, const std::filesystem::path&) override
    {
        return std::unexpected("injected catalog replace failure");
    }
    void Remove(const std::filesystem::path& path) noexcept override
    {
        std::error_code ignored;
        std::filesystem::remove(path, ignored);
    }
};
}

int main()
{
    using namespace Aether;
    using namespace Aether::GameFeatures;
    using namespace Aether::ProjectAssets;
    using namespace Aether::EditorFramework;

    const auto root = std::filesystem::temp_directory_path() / ("aether-asset-document-" + ProjectId::Create().ToString());
    std::filesystem::create_directories(root);
    const auto project = ProjectId::Create();
    ProjectManifest manifest{project, {{"sample.feature", 1}}};
    CompiledFeature feature;
    feature.descriptor = {"sample.feature", 1, 1, {}};
    feature.registerRuntime = [](RuntimeFeatureRegistrar& registrar) -> FeatureResult<void> {
        AssetTypeDescriptor type;
        type.id = "sample.palette";
        type.ownerFeature = "sample.feature";
        type.loadMetadata = [](const std::filesystem::path& path) -> Result<Json> {
            std::ifstream stream(path);
            try { return Json::parse(stream); }
            catch (...) { return std::unexpected(Error{ErrorCode::InvalidFormat, "palette metadata is invalid"}); }
        };
        auto registered = registrar.RegisterAssetType(std::move(type));
        if (!registered) return std::unexpected(FeatureError{registrar.Feature().id, registered.error().message});
        return {};
    };
    auto runtime = RuntimeRegistry::Build(manifest, {feature});
    assert(runtime);
    auto emptyCatalog = CatalogSnapshot::Create({project, 0, {}}, (*runtime)->AssetTypes());
    assert(emptyCatalog);

    const auto input = root / "outside.palette";
    { std::ofstream stream(input); stream << "{}"; }
    AssetImporterDescriptor importer;
    importer.id = "sample.palette-importer";
    importer.ownerFeature = "sample.feature";
    importer.outputType = "sample.palette";
    importer.outputFormatVersion = 1;
    importer.extensions = {".palette"};
    importer.import = [](const std::vector<ImportInputData>&, const Json&) -> std::expected<AssetImportProduct, std::string> {
        AssetImportProduct product;
        product.displayPath = "Palettes/Main";
        product.entryPoint = "Artifacts/palette.json";
        product.artifacts.push_back({product.entryPoint, Bytes("{\"value\":1}")});
        return product;
    };
    AssetImportRequest importRequest;
    importRequest.projectId = project;
    importRequest.sessionGeneration = 7;
    importRequest.catalogGeneration = 0;
    importRequest.importerId = importer.id;
    importRequest.inputs = {{"palette.json", input}};
    importRequest.displayPath = "Palettes/Main";
    auto importTask = StartAssetImport(importRequest, importer, root);
    auto preparedImport = importTask.result.get();
    assert(preparedImport);
    AssetImportCommitContext context{project, 7, false, true};
    auto initialCatalog = CommitAssetImport(std::move(*preparedImport), context, **emptyCatalog, **runtime, root);
    assert(initialCatalog && (*initialCatalog)->Generation() == 1);
    const auto* imported = (*initialCatalog)->Find((*initialCatalog)->Records().front().id);
    assert(imported && imported->revision == 1);
    const auto assetId = imported->id;
    std::filesystem::remove(input);

    AssetEditorDescriptor editor;
    editor.ownerFeature = "sample.feature";
    editor.supportedTypes = {"sample.palette"};
    editor.editorId = "sample.palette-editor";
    editor.isDefault = true;
    editor.createDocument = [](World& world, EntityId entity, const AssetRecord& record) -> std::expected<void, std::string> {
        world.AddComponent<DraftValue>(entity, DraftValue{record.revision, record.displayPath});
        return {};
    };
    editor.prepareSave = [](const World& world, EntityId entity) -> std::expected<AssetImportProduct, std::string> {
        const auto& draft = world.GetComponent<DraftValue>(entity);
        const auto json = Json{{"value", draft.value}}.dump();
        AssetImportProduct product;
        product.displayPath = draft.displayPath;
        product.entryPoint = "Artifacts/palette.json";
        product.settings = Json{{"savedValue", draft.value}};
        product.sources.push_back({"Source/palette.json", Bytes(json)});
        product.artifacts.push_back({product.entryPoint, Bytes(json)});
        return product;
    };
    AssetEditorRegistry editors;
    assert(editors.Register(editor, **runtime));
    editors.Freeze();
    World editorWorld;
    auto opened = OpenAssetDocument(editorWorld, **initialCatalog, editors, {project, assetId});
    assert(opened && !opened->readOnly && !opened->reused);
    auto& draft = editorWorld.GetComponent<DraftValue>(opened->entity);
    draft.value = 2;
    editorWorld.GetComponent<DocumentComponent>(opened->entity).dirty = true;
    editorWorld.GetComponent<HistoryComponent>(opened->entity).currentState = 1;

    auto saveTask = PrepareAssetDocumentSave(editorWorld, opened->entity, editors, **runtime, **initialCatalog, 7, root);
    assert(saveTask);
    auto preparedSave = saveTask->result.get();
    assert(preparedSave);
    auto savedCatalog = CommitAssetDocumentSave(editorWorld, opened->entity, std::move(*preparedSave), context,
        **initialCatalog, **runtime, root);
    assert(savedCatalog && (*savedCatalog)->Generation() == 2);
    auto& assetDocument = editorWorld.GetComponent<AssetDocumentComponent>(opened->entity);
    assert(assetDocument.baseRevision == 2 && !editorWorld.GetComponent<DocumentComponent>(opened->entity).dirty);
    assert(editorWorld.GetComponent<HistoryComponent>(opened->entity).savedState == 1);
    const auto* savedRecord = (*savedCatalog)->Find(assetId);
    assert(savedRecord && savedRecord->revision == 2);
    auto resolved = ResolveAsset(**savedCatalog, {project, assetId}, "sample.palette", root,
        (*runtime)->AssetTypes(), "asset-document-test");
    assert(resolved && std::filesystem::exists(resolved->entryPath));
    auto reopened = OpenAssetDocument(editorWorld, **savedCatalog, editors, {project, assetId});
    assert(reopened && reopened->reused && reopened->entity == opened->entity);

    draft.value = 3;
    editorWorld.GetComponent<DocumentComponent>(opened->entity).dirty = true;
    editorWorld.GetComponent<HistoryComponent>(opened->entity).currentState = 2;
    auto secondTask = PrepareAssetDocumentSave(editorWorld, opened->entity, editors, **runtime, **savedCatalog, 7, root);
    assert(secondTask);
    auto secondPrepared = secondTask->result.get();
    assert(secondPrepared);
    auto playLocked = context;
    playLocked.playLocked = true;
    auto rejectedPlayCommit = CommitAssetDocumentSave(editorWorld, opened->entity, std::move(*secondPrepared), playLocked,
        **savedCatalog, **runtime, root);
    assert(!rejectedPlayCommit && editorWorld.GetComponent<DocumentComponent>(opened->entity).dirty);
    assert(editorWorld.GetComponent<AssetDocumentComponent>(opened->entity).baseRevision == 2);

    auto thirdTask = PrepareAssetDocumentSave(editorWorld, opened->entity, editors, **runtime, **savedCatalog, 7, root);
    assert(thirdTask);
    auto thirdPrepared = thirdTask->result.get();
    assert(thirdPrepared);
    FailReplace failReplace;
    auto failedSave = CommitAssetDocumentSave(editorWorld, opened->entity, std::move(*thirdPrepared), context,
        **savedCatalog, **runtime, root, &failReplace);
    assert(!failedSave && (*savedCatalog)->Generation() == 2);
    assert(editorWorld.GetComponent<DocumentComponent>(opened->entity).dirty);
    auto currentResolved = ResolveAsset(**savedCatalog, {project, assetId}, "sample.palette", root,
        (*runtime)->AssetTypes(), "asset-document-test");
    assert(currentResolved && std::filesystem::exists(currentResolved->entryPath));
    std::filesystem::remove_all(root);
}
