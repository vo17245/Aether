#include <EditorFramework/Storage/AtomicReplaceFile.h>
#include <EditorFramework/AssetImport.h>
#include <GameFeature/RuntimeFeatureRegistrar.h>

#include <cassert>
#include <fstream>
#include <iterator>

namespace
{
enum class FailAt { None, Write, Flush, Replace };
class FakeOperations final : public Aether::EditorFramework::Storage::AtomicFileOperations
{
public:
    FailAt failAt = FailAt::None;
    Aether::EditorFramework::Storage::FileResult Write(const std::filesystem::path& path,
                                                       std::span<const std::byte> bytes) override
    {
        temporary = path;
        std::ofstream stream(path, std::ios::binary);
        stream.write(reinterpret_cast<const char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
        stream.close();
        if (failAt == FailAt::Write) return std::unexpected("injected write failure");
        return {};
    }
    Aether::EditorFramework::Storage::FileResult Flush(const std::filesystem::path&) override
    {
        if (failAt == FailAt::Flush) return std::unexpected("injected flush failure");
        return {};
    }
    Aether::EditorFramework::Storage::FileResult Replace(const std::filesystem::path& from,
                                                         const std::filesystem::path& to) override
    {
        if (failAt == FailAt::Replace) return std::unexpected("injected replace failure");
        std::filesystem::rename(from, to);
        return {};
    }
    void Remove(const std::filesystem::path& path) noexcept override
    {
        std::error_code ignored;
        std::filesystem::remove(path, ignored);
    }
    std::filesystem::path temporary;
};

std::string Read(const std::filesystem::path& path)
{
    std::ifstream stream(path, std::ios::binary);
    return {std::istreambuf_iterator<char>(stream), std::istreambuf_iterator<char>()};
}
}

int main()
{
    using namespace Aether;
    using namespace Aether::EditorFramework;
    using namespace Aether::EditorFramework::Storage;
    const auto root = std::filesystem::temp_directory_path() / "aether-atomic-replace-test";
    std::filesystem::remove_all(root);
    std::filesystem::create_directories(root);
    const auto destination = root / "catalog.json";
    { std::ofstream stream(destination); stream << "old"; }
    const std::string next = "new contents";
    const auto bytes = std::as_bytes(std::span(next.data(), next.size()));

    for (const auto failAt : {FailAt::Write, FailAt::Flush, FailAt::Replace})
    {
        FakeOperations fake;
        fake.failAt = failAt;
        auto result = AtomicReplaceFile(destination, bytes, &fake);
        assert(!result);
        assert(Read(destination) == "old");
        assert(!std::filesystem::exists(fake.temporary));
    }

    assert(AtomicReplaceFile(destination, bytes));
    assert(Read(destination) == next);

    using namespace Aether::GameFeatures;
    using namespace Aether::ProjectAssets;
    const auto project = ProjectId::Create();
    ProjectManifest manifest{project, {{"sample.runtime", 1}}};
    CompiledFeature feature;
    feature.descriptor = {"sample.runtime", 1, 1, {}};
    feature.registerRuntime = [](RuntimeFeatureRegistrar& registrar) -> FeatureResult<void> {
        AssetTypeDescriptor type;
        type.id = "sample.mesh";
        type.ownerFeature = "sample.runtime";
        type.loadMetadata = [](const std::filesystem::path& path) -> Result<Json> {
            std::ifstream stream(path);
            try { return Json::parse(stream); }
            catch (...) { return std::unexpected(Error{ErrorCode::InvalidFormat, "mesh artifact is not JSON"}); }
        };
        type.enumerateDependencies = [](const std::filesystem::path&) -> Result<std::vector<ProjectAssetRef>> {
            return std::vector<ProjectAssetRef>{};
        };
        auto registered = registrar.RegisterAssetType(std::move(type));
        if (!registered) return std::unexpected(FeatureError{registrar.Feature().id, registered.error().message});
        return {};
    };
    auto runtime = RuntimeRegistry::Build(manifest, {feature});
    assert(runtime);
    AssetImporterDescriptor importer;
    importer.id = "sample.mesh-importer";
    importer.ownerFeature = "sample.runtime";
    importer.outputType = "sample.mesh";
    importer.extensions = {".mesh"};
    importer.probe = [](std::span<const std::byte> leading) {
        return leading.size() >= 2 && leading[0] == std::byte{'O'} && leading[1] == std::byte{'K'};
    };
    importer.import = [](const std::vector<ImportInputData>& inputs, const Json&) -> std::expected<AssetImportProduct, std::string> {
        if (inputs.empty() || inputs.front().contents.empty()) return std::unexpected(std::string("missing importer input"));
        const std::string contents = "{}";
        AssetImportProduct product;
        product.displayPath = "Meshes/Imported";
        product.entryPoint = "Artifacts/mesh.json";
        product.artifacts.push_back({product.entryPoint, std::vector<std::byte>(
            reinterpret_cast<const std::byte*>(contents.data()), reinterpret_cast<const std::byte*>(contents.data() + contents.size()))});
        return product;
    };
    AssetImporterRegistry importers;
    assert(importers.Register(importer, **runtime));
    assert(!importers.Register(importer, **runtime));
    importers.Freeze();
    assert(!importers.Register(importer, **runtime));

    const auto importRoot = root / "project";
    std::filesystem::create_directories(importRoot);
    const auto sourcePath = root / "input.mesh";
    { std::ofstream stream(sourcePath, std::ios::binary); stream << "OK external source"; }
    const std::string leading = "OK";
    const auto leadingBytes = std::as_bytes(std::span(leading.data(), leading.size()));
    const auto candidates = importers.Candidates(sourcePath, leadingBytes);
    assert(candidates.size() == 1 && candidates.front() == importer.id);
    auto initialCatalog = CatalogSnapshot::Create({project, 0, {}}, (*runtime)->AssetTypes());
    assert(initialCatalog);
    AssetImportRequest request;
    request.projectId = project;
    request.sessionGeneration = 5;
    request.catalogGeneration = 0;
    request.importerId = importer.id;
    request.inputs.push_back({"primary.mesh", sourcePath});
    request.settings = Json{{"scale", 1.0}};
    request.displayPath = "Meshes/Imported";
    auto boundedImporter = *importers.Find(importer.id);
    boundedImporter.maxInputBytes = 4;
    auto oversizedTask = StartAssetImport(request, boundedImporter, importRoot);
    auto oversizedPrepared = oversizedTask.result.get();
    assert(!oversizedPrepared);
    assert(oversizedPrepared.error().find("size limits") != std::string::npos);

    auto task = StartAssetImport(request, *importers.Find(importer.id), importRoot);
    auto prepared = task.result.get();
    assert(prepared);
    const auto importedAssetId = prepared->record.id;
    const auto stage = prepared->stagingDirectory;
    task.Cancel();
    AssetImportCommitContext commitContext{project, 5, false, true};
    auto cancelledCommit = CommitAssetImport(std::move(*prepared), commitContext, **initialCatalog, **runtime, importRoot);
    assert(!cancelledCommit && !std::filesystem::exists(stage));

    auto validTask = StartAssetImport(request, *importers.Find(importer.id), importRoot);
    auto validPrepared = validTask.result.get();
    assert(validPrepared && validPrepared->record.id != importedAssetId);
    auto staleContext = commitContext;
    staleContext.currentSessionGeneration = 6;
    auto staleCommit = CommitAssetImport(std::move(*validPrepared), staleContext, **initialCatalog, **runtime, importRoot);
    assert(!staleCommit);

    auto successfulTask = StartAssetImport(request, *importers.Find(importer.id), importRoot);
    auto successfulPrepared = successfulTask.result.get();
    assert(successfulPrepared);
    const auto committedId = successfulPrepared->record.id;
    auto committedCatalog = CommitAssetImport(std::move(*successfulPrepared), commitContext, **initialCatalog, **runtime, importRoot);
    assert(committedCatalog && (*committedCatalog)->Generation() == 1);
    std::filesystem::remove(sourcePath);
    auto resolved = ResolveAsset(**committedCatalog, {project, committedId}, "sample.mesh", importRoot,
        (*runtime)->AssetTypes(), "test.asset");
    assert(resolved && std::filesystem::exists(resolved->entryPath));

    AssetRenameRequest rename{project, 5, 1, committedId, "Meshes/Renamed"};
    auto renamedCatalog = CommitAssetRename(rename, commitContext, **committedCatalog, **runtime, importRoot);
    assert(renamedCatalog && (*renamedCatalog)->Generation() == 2);
    assert((*renamedCatalog)->Find(committedId)->displayPath == "Meshes/Renamed");
    assert((*renamedCatalog)->Find(committedId)->revision == 1);
    auto renameResolved = ResolveAsset(**renamedCatalog, {project, committedId}, "sample.mesh", importRoot,
        (*runtime)->AssetTypes(), "test.renamed");
    assert(renameResolved && renameResolved->entryPath == resolved->entryPath);
    assert(!CommitAssetRename(rename, AssetImportCommitContext{project, 5, true, true},
        **renamedCatalog, **runtime, importRoot));
    auto staleRename = rename;
    staleRename.catalogGeneration = 1;
    assert(!CommitAssetRename(staleRename, commitContext, **renamedCatalog, **runtime, importRoot));
    auto unsafeRename = rename;
    unsafeRename.catalogGeneration = 2;
    unsafeRename.displayPath = "../outside";
    assert(!CommitAssetRename(unsafeRename, commitContext, **renamedCatalog, **runtime, importRoot));
    auto collisionRename = rename;
    collisionRename.catalogGeneration = 2;
    collisionRename.displayPath = "Meshes/Renamed/../Other";
    assert(!CommitAssetRename(collisionRename, commitContext, **renamedCatalog, **runtime, importRoot));
    FakeOperations failRenameReplace;
    failRenameReplace.failAt = FailAt::Replace;
    auto failedRename = rename;
    failedRename.catalogGeneration = 2;
    failedRename.displayPath = "Meshes/Failed";
    assert(!CommitAssetRename(failedRename, commitContext, **renamedCatalog, **runtime, importRoot,
        &failRenameReplace));
    auto persistedAfterFailedRename = DecodeCatalog(Aether::Json::parse(Read(importRoot / "Assets" / "catalog.json")));
    assert(persistedAfterFailedRename && persistedAfterFailedRename->generation == 2
        && persistedAfterFailedRename->records.front().displayPath == "Meshes/Renamed");

    auto reimport = request;
    reimport.catalogGeneration = 2;
    reimport.existingAsset = committedId;
    reimport.baseRevision = 1;
    const auto replacementSource = root / "replacement.mesh";
    { std::ofstream stream(replacementSource, std::ios::binary); stream << "OK replacement"; }
    reimport.inputs = {{"primary.mesh", replacementSource}};
    auto reimportTask = StartAssetImport(reimport, *importers.Find(importer.id), importRoot);
    auto reimportPrepared = reimportTask.result.get();
    assert(reimportPrepared);
    FakeOperations failCatalogReplace;
    failCatalogReplace.failAt = FailAt::Replace;
    auto failedCommit = CommitAssetImport(std::move(*reimportPrepared), commitContext, **renamedCatalog,
        **runtime, importRoot, &failCatalogReplace);
    assert(!failedCommit && (*renamedCatalog)->Generation() == 2);
    auto persistedCatalogJson = Aether::Json::parse(Read(importRoot / "Assets" / "catalog.json"));
    auto persistedCatalog = DecodeCatalog(persistedCatalogJson);
    assert(persistedCatalog && persistedCatalog->generation == 2 && persistedCatalog->records.size() == 1
        && persistedCatalog->records.front().displayPath == "Meshes/Renamed");

    std::filesystem::remove_all(importRoot);
    std::filesystem::remove(replacementSource);
    std::filesystem::remove_all(root);
}
