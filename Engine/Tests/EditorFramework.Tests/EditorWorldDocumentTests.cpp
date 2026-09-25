#include <EditorFramework/WorldDocument.h>
#include <GameFeature/ProjectWorld.h>

#include <cassert>
#include <fstream>

namespace
{
class FailReplace final : public Aether::EditorFramework::Storage::AtomicFileOperations
{
public:
    Aether::EditorFramework::Storage::FileResult Write(const std::filesystem::path& path,
        std::span<const std::byte> bytes) override
    {
        std::ofstream stream(path, std::ios::binary);
        stream.write(reinterpret_cast<const char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
        return stream ? Aether::EditorFramework::Storage::FileResult{} : std::unexpected("write failed");
    }
    Aether::EditorFramework::Storage::FileResult Flush(const std::filesystem::path&) override { return {}; }
    Aether::EditorFramework::Storage::FileResult Replace(const std::filesystem::path&, const std::filesystem::path&) override
    {
        return std::unexpected("injected replace failure");
    }
    void Remove(const std::filesystem::path& path) noexcept override
    {
        std::error_code ignored;
        std::filesystem::remove(path, ignored);
    }
};

std::string ReadFile(const std::filesystem::path& path)
{
    std::ifstream stream(path, std::ios::binary);
    return {std::istreambuf_iterator<char>(stream), std::istreambuf_iterator<char>()};
}
}

int main()
{
    using namespace Aether;
    using namespace Aether::GameFeatures;
    using namespace Aether::EditorFramework;
    const auto projectId = ProjectAssets::ProjectId::Create();
    const auto documentId = DocumentId::Create();
    ProjectAssets::ProjectManifest manifest{projectId, {{"aether.base", 1}}};
    CompiledFeature base;
    base.descriptor = {"aether.base", 1, 1, {}};
    base.registerRuntime = [](RuntimeFeatureRegistrar& registrar) { return RegisterBaseRuntimeComponents(registrar); };
    auto runtime = RuntimeRegistry::Build(manifest, {base});
    assert(runtime);
    auto catalog = ProjectAssets::CatalogSnapshot::Create({projectId, 0, {}}, (*runtime)->AssetTypes());
    assert(catalog);

    World authored;
    const auto parent = authored.CreateEntity();
    const auto parentId = PersistentEntityId::Create();
    authored.AddComponent<PersistentEntityIdComponent>(parent, PersistentEntityIdComponent{parentId});
    authored.AddComponent<EntityNameComponent>(parent, EntityNameComponent{"Parent"});
    const auto child = authored.CreateEntity();
    const auto childId = PersistentEntityId::Create();
    authored.AddComponent<PersistentEntityIdComponent>(child, PersistentEntityIdComponent{childId});
    authored.AddComponent<EntityNameComponent>(child, EntityNameComponent{"Child"});
    authored.AddComponent<ParentComponent>(child, ParentComponent{parentId});
    const auto service = authored.CreateEntity();
    authored.AddComponent<Serialization::ExcludeFromArchiveComponent>(service);

    auto encoded = EncodeProjectWorld(authored, documentId, **runtime, **catalog, std::filesystem::temp_directory_path());
    assert(encoded);
    auto decoded = DecodeProjectWorld(*encoded, **runtime, **catalog, std::filesystem::temp_directory_path());
    assert(decoded && decoded->documentId == documentId);
    assert(decoded->world->Entities().size() == 2);
    bool foundChild = false;
    for (const auto entity : decoded->world->Entities())
    {
        assert(decoded->world->HasComponent<PersistentEntityIdComponent>(entity));
        const auto& id = decoded->world->GetComponent<PersistentEntityIdComponent>(entity).value;
        if (id == childId)
        {
            foundChild = true;
            assert(decoded->world->GetComponent<ParentComponent>(entity).parent == parentId);
            assert(decoded->world->GetComponent<EntityNameComponent>(entity).value == "Child");
        }
    }
    assert(foundChild);

    auto clone = CloneProjectWorldForPlay(authored, documentId, **runtime, **catalog, std::filesystem::temp_directory_path());
    assert(clone && clone->world->Entities().size() == 2);

    World invalid;
    invalid.CreateEntity();
    assert(!EncodeProjectWorld(invalid, DocumentId::Create(), **runtime, **catalog, std::filesystem::temp_directory_path()));

    World duplicate;
    auto first = duplicate.CreateEntity();
    auto second = duplicate.CreateEntity();
    duplicate.AddComponent<PersistentEntityIdComponent>(first, PersistentEntityIdComponent{parentId});
    duplicate.AddComponent<PersistentEntityIdComponent>(second, PersistentEntityIdComponent{parentId});
    assert(!EncodeProjectWorld(duplicate, DocumentId::Create(), **runtime, **catalog, std::filesystem::temp_directory_path()));

    const auto projectRoot = std::filesystem::temp_directory_path() / ("aether-world-document-" + projectId.ToString());
    std::filesystem::remove_all(projectRoot);
    assert(EditorFramework::SaveProjectWorldFile(authored, documentId, projectRoot, **runtime, **catalog));
    const auto worldPath = projectRoot / "Worlds" / (documentId.ToString() + ".world.json");
    auto fileDecoded = EditorFramework::LoadProjectWorldFile(worldPath, projectRoot, **runtime, **catalog);
    assert(fileDecoded && fileDecoded->documentId == documentId && fileDecoded->world->Entities().size() == 2);
    World editorWorld;
    const auto sessionEntity = editorWorld.CreateEntity();
    editorWorld.AddComponent<EditorSessionComponent>(sessionEntity);
    auto openedDocument = EditorFramework::OpenProjectWorldDocument(editorWorld, worldPath, projectRoot,
        *runtime, *catalog);
    assert(openedDocument && openedDocument->documentId == documentId);
    assert(editorWorld.HasComponent<HistoryComponent>(openedDocument->entity));
    assert(!editorWorld.GetComponent<DocumentComponent>(openedDocument->entity).dirty);
    assert(editorWorld.GetComponent<WorldDocumentComponent>(openedDocument->entity).authoringWorld->Entities().size() == 2);
    assert(editorWorld.GetComponent<SelectionComponent>(sessionEntity).world == openedDocument->worldInstanceId);
    assert(!EditorFramework::OpenProjectWorldDocument(editorWorld, worldPath, projectRoot, *runtime, *catalog));
    editorWorld.DestroyEntity(openedDocument->entity);
    auto reopenedDocument = EditorFramework::OpenProjectWorldDocument(editorWorld, worldPath, projectRoot,
        *runtime, *catalog);
    assert(reopenedDocument && reopenedDocument->documentId == documentId);
    bool reopenedChildFound = false;
    const auto& reopenedWorld = *editorWorld.GetComponent<WorldDocumentComponent>(reopenedDocument->entity).authoringWorld;
    for (const auto entity : reopenedWorld.Select<PersistentEntityIdComponent>())
        if (reopenedWorld.GetComponent<PersistentEntityIdComponent>(entity).value == childId)
            reopenedChildFound = reopenedWorld.GetComponent<ParentComponent>(entity).parent == parentId;
    assert(reopenedChildFound);
    const auto previousFile = ReadFile(worldPath);
    FailReplace failReplace;
    authored.GetComponent<EntityNameComponent>(parent).value = "Changed";
    assert(!EditorFramework::SaveProjectWorldFile(authored, documentId, projectRoot, **runtime, **catalog, &failReplace));
    assert(ReadFile(worldPath) == previousFile);
    auto stillReadable = EditorFramework::LoadProjectWorldFile(worldPath, projectRoot, **runtime, **catalog);
    assert(stillReadable && stillReadable->world->Entities().size() == 2);
    const auto outside = std::filesystem::temp_directory_path() / "aether-world-outside.json";
    { std::ofstream stream(outside); stream << "{}"; }
    assert(!EditorFramework::LoadProjectWorldFile(outside, projectRoot, **runtime, **catalog));
    std::filesystem::remove(outside);
    std::filesystem::remove_all(projectRoot);
}
