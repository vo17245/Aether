#include <EditorFramework/ProjectLifecycle.h>
#include <ProjectAsset/ProjectManifest.h>

#include <cassert>
#include <fstream>

namespace
{
void WriteJson(const std::filesystem::path& path, const Aether::Json& json)
{
    std::filesystem::create_directories(path.parent_path());
    std::ofstream stream(path, std::ios::binary);
    stream << json.dump(2);
}
}

int main()
{
    using namespace Aether;
    using namespace Aether::GameFeatures;
    using namespace Aether::ProjectAssets;
    using namespace Aether::EditorFramework;

    const auto root = std::filesystem::temp_directory_path() / ("aether-project-context-" + ProjectId::Create().ToString());
    std::filesystem::create_directories(root);
    const auto projectId = ProjectId::Create();
    ProjectManifest manifest{projectId, {{"sample.runtime", 1}}};
    CompiledFeature feature;
    feature.descriptor = {"sample.runtime", 1, 1, {}};
    feature.registerRuntime = [](RuntimeFeatureRegistrar&) { return FeatureResult<void>{}; };
    auto runtime = RuntimeRegistry::Build(manifest, {feature});
    assert(runtime);
    auto encodedManifest = EncodeProjectManifest(manifest);
    assert(encodedManifest);
    auto catalog = CatalogSnapshot::Create({projectId, 0, {}}, (*runtime)->AssetTypes());
    assert(catalog);
    auto encodedCatalog = EncodeCatalog(**catalog);
    assert(encodedCatalog);
    WriteJson(root / "project.aether.json", *encodedManifest);
    WriteJson(root / "Assets" / "catalog.json", *encodedCatalog);

    auto importers = std::make_shared<AssetImporterRegistry>();
    importers->Freeze();
    ProjectStateComponent first;
    EditorServicesComponent firstServices;
    assert(OpenProjectContext(first, firstServices, root, *runtime, importers, true));
    assert(first.open && first.writable && first.writeLock && first.projectId == projectId);
    assert(first.catalog && firstServices.catalog == first.catalog && firstServices.runtime == *runtime);
    const auto generation = first.sessionGeneration;

    const auto invalidRoot = root / "missing";
    assert(!OpenProjectContext(first, firstServices, invalidRoot, *runtime, importers, true));
    assert(first.open && first.projectId == projectId && first.sessionGeneration == generation);
    assert(firstServices.projectRoot == std::filesystem::weakly_canonical(root));

    ProjectStateComponent second;
    EditorServicesComponent secondServices;
    assert(!OpenProjectContext(second, secondServices, root, *runtime, importers, true));
    assert(!second.open && !second.writeLock);

    assert(!CloseProjectContext(first, firstServices, {.dirtyDocuments = true}));
    assert(!CloseProjectContext(first, firstServices, {.playActive = true}));
    assert(first.open && first.writeLock);
    assert(CloseProjectContext(first, firstServices, {.dirtyDocuments = true, .discardDirtyDocuments = true}));
    assert(!first.open && !first.writeLock && !first.catalog && !firstServices.runtime && !firstServices.catalog);
    assert(first.sessionGeneration == generation + 1);

    assert(OpenProjectContext(second, secondServices, root, *runtime, importers, true));
    assert(second.writeLock && secondServices.catalog);
    assert(CloseProjectContext(second, secondServices, {}));
    std::filesystem::remove_all(root);
}
