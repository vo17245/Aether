#include <ProjectAsset/AssetResolver.h>
#include <ProjectAsset/ProjectLock.h>
#include <ProjectAsset/ProjectManifest.h>

#include <cassert>
#include <fstream>

int main()
{
    using namespace Aether::ProjectAssets;
    const auto project = ProjectId::Create();
    ProjectManifest manifest{project, {{"sample.scene", 1}}};
    auto encodedManifest = EncodeProjectManifest(manifest);
    assert(encodedManifest);
    auto decodedManifest = DecodeProjectManifest(*encodedManifest);
    assert(decodedManifest && decodedManifest->projectId == project && decodedManifest->features.size() == 1);
    auto malformedManifest = *encodedManifest;
    malformedManifest["projectId"] = "not-a-uuid";
    assert(!DecodeProjectManifest(malformedManifest));

    AssetTypeRegistry types;
    AssetTypeDescriptor sceneType;
    sceneType.id = "sample.scene";
    sceneType.ownerFeature = "sample.runtime";
    sceneType.formatVersion = 1;
    assert(types.Register(std::move(sceneType)));
    types.Freeze();

    const auto asset = AssetId::Create();
    const auto root = std::filesystem::temp_directory_path() / ("aether-project-assets-" + project.ToString());
    const auto artifactRoot = std::filesystem::path("Assets/Data") / asset.ToString() / "1";
    const auto entry = std::filesystem::path("Artifacts/scene.json");
    std::filesystem::create_directories(root / artifactRoot / entry.parent_path());
    auto writeLock = ProjectWriteLock::Acquire(root);
    assert(writeLock);
    assert(!ProjectWriteLock::Acquire(root));
    {
        std::ofstream stream(root / artifactRoot / entry, std::ios::binary);
        stream << "{}";
    }

    AssetRecord record;
    record.id = asset;
    record.type = "sample.scene";
    record.ownerFeature = "sample.runtime";
    record.formatVersion = 1;
    record.revision = 1;
    record.displayPath = "Scenes/Main";
    record.artifactRoot = artifactRoot.generic_string();
    record.entryPoint = entry.generic_string();
    record.files.push_back({record.entryPoint, 2});
    record.provenance.importerId = "sample.scene-importer";
    record.provenance.importerVersion = 1;
    auto catalog = CatalogSnapshot::Create({project, 7, {record}}, types);
    assert(catalog);
    auto encodedCatalog = EncodeCatalog(**catalog);
    assert(encodedCatalog);
    auto decodedCatalog = DecodeCatalog(*encodedCatalog);
    assert(decodedCatalog && decodedCatalog->generation == 7 && decodedCatalog->records.size() == 1);
    auto roundTrip = CatalogSnapshot::Create(std::move(*decodedCatalog), types);
    assert(roundTrip);

    ProjectAssetRef ref{project, asset};
    auto resolved = ResolveAsset(**roundTrip, ref, "sample.scene", root, types, "scene.settings");
    assert(resolved && resolved->entryPath == std::filesystem::weakly_canonical(root / artifactRoot / entry));
    assert(ValidateReferenceClosure(**roundTrip, ref, root, types));
    assert(!ResolveAsset(**roundTrip, ref, "sample.texture", root, types, "scene.texture"));
    assert(!ResolveAsset(**roundTrip, {ProjectId::Create(), asset}, "sample.scene", root, types));

    auto corruptRecord = record;
    corruptRecord.files[0].path = "../../outside.json";
    assert(!CatalogSnapshot::Create({project, 8, {corruptRecord}}, types));
    writeLock->reset();
    auto replacementLock = ProjectWriteLock::Acquire(root);
    assert(replacementLock);
    replacementLock->reset();
    std::filesystem::remove_all(root);
}
