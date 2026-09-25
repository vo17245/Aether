#include <EditorFramework/Commands.h>

#include <cassert>
#include <algorithm>

namespace
{
struct MoveOnlyValue { std::unique_ptr<int> value; };
struct EntityReference { Aether::GameFeatures::PersistentEntityId target; };
}

namespace Aether::Serialization
{
template<> struct ComponentSerialization<MoveOnlyValue>
{
    static constexpr std::string_view Type = "test.move-only";
    static constexpr std::uint32_t Version = 1;
    static Result<Json> Serialize(const MoveOnlyValue& value, SaveContext&)
    {
        if (!value.value) return std::unexpected(ArchiveError{ArchiveErrorCode::InvalidFormat, "missing move-only value"});
        return Json(*value.value);
    }
    static Result<MoveOnlyValue> Deserialize(const Json& json, std::uint32_t version, LoadContext&)
    {
        if (version != 1 || !json.is_number_integer()) return std::unexpected(ArchiveError{ArchiveErrorCode::InvalidFormat, "invalid move-only value"});
        return MoveOnlyValue{std::make_unique<int>(json.get<int>())};
    }
};

template<> struct ComponentSerialization<EntityReference>
{
    static constexpr std::string_view Type = "test.entity-reference";
    static constexpr std::uint32_t Version = 1;
    static Result<Json> Serialize(const EntityReference& value, SaveContext&)
    {
        return Json(value.target.ToString());
    }
    static Result<EntityReference> Deserialize(const Json& json, std::uint32_t version, LoadContext&)
    {
        if (version != 1 || !json.is_string()) return std::unexpected(ArchiveError{ArchiveErrorCode::InvalidFormat, "invalid entity reference"});
        auto target = GameFeatures::PersistentEntityId::Parse(json.get<std::string>());
        if (!target) return std::unexpected(ArchiveError{ArchiveErrorCode::InvalidFormat, "invalid entity reference ID"});
        return EntityReference{*target};
    }
};
}

namespace
{
void Apply(Aether::World& editorWorld)
{
    Aether::EditorFramework::ApplyEditCommands(editorWorld);
}

const Aether::EditorFramework::EditCommandResultComponent& Result(Aether::World& editorWorld, Aether::EntityId request)
{
    return editorWorld.GetComponent<Aether::EditorFramework::EditCommandResultComponent>(request);
}

Aether::EntityId FindById(Aether::World& world, const Aether::GameFeatures::PersistentEntityId& id)
{
    for (const auto entity : world.Select<Aether::GameFeatures::PersistentEntityIdComponent>())
        if (world.GetComponent<Aether::GameFeatures::PersistentEntityIdComponent>(entity).value == id) return entity;
    return entt::null;
}
}

int main()
{
    using namespace Aether;
    using namespace Aether::EditorFramework;
    using namespace Aether::GameFeatures;

    ProjectAssets::ProjectManifest manifest{ProjectAssets::ProjectId::Create(), {{"test.commands", 1}}};
    CompiledFeature feature;
    feature.descriptor = {"test.commands", 1, 1, {}};
    feature.registerRuntime = [](RuntimeFeatureRegistrar& registrar) -> FeatureResult<void> {
        auto base = RegisterBaseRuntimeComponents(registrar);
        if (!base) return base;
        auto moveOnly = registrar.RegisterComponent<MoveOnlyValue>();
        if (!moveOnly) return moveOnly;
        return registrar.RegisterComponent<EntityReference>({}, [](const EntityReference& reference, const auto& visit) {
            visit(reference.target);
        });
    };
    auto runtime = RuntimeRegistry::Build(manifest, {feature});
    assert(runtime);
    auto catalog = ProjectAssets::CatalogSnapshot::Create({manifest.projectId, 0, {}}, (*runtime)->AssetTypes());
    assert(catalog);

    auto commands = std::make_shared<CommandRegistry>();
    assert(RegisterCoreEntityCommands(*commands, *runtime));
    commands->Freeze();

    const auto documentId = DocumentId::Create();
    const auto worldInstance = WorldInstanceId::Create();
    auto authoring = std::make_unique<World>();
    const auto rootId = PersistentEntityId::Create();
    const auto root = authoring->CreateEntity();
    authoring->AddComponent<PersistentEntityIdComponent>(root, PersistentEntityIdComponent{rootId});
    authoring->AddComponent<EntityNameComponent>(root, EntityNameComponent{"Root"});
    authoring->AddComponent<MoveOnlyValue>(root, MoveOnlyValue{std::make_unique<int>(42)});
    const auto referencing = authoring->CreateEntity();
    authoring->AddComponent<PersistentEntityIdComponent>(referencing, PersistentEntityIdComponent{PersistentEntityId::Create()});
    authoring->AddComponent<EntityNameComponent>(referencing, EntityNameComponent{"Reference"});
    authoring->AddComponent<EntityReference>(referencing, EntityReference{rootId});

    World editorWorld;
    const auto services = editorWorld.CreateEntity();
    editorWorld.AddComponent<EditorServicesComponent>(services, EditorServicesComponent{commands, *runtime, *catalog, std::filesystem::temp_directory_path()});
    const auto documentEntity = editorWorld.CreateEntity();
    editorWorld.AddComponent<DocumentComponent>(documentEntity, DocumentComponent{documentId, DocumentKind::World, false, {}});
    WorldDocumentComponent worldDocument;
    worldDocument.authoringWorld = std::move(authoring);
    worldDocument.instanceId = worldInstance;
    editorWorld.AddComponent<WorldDocumentComponent>(documentEntity, std::move(worldDocument));
    editorWorld.AddComponent<HistoryComponent>(documentEntity);

    const auto createdId = PersistentEntityId::Create();
    const auto invalidBatch = SubmitEditCommand(editorWorld, documentId, worldInstance, {
        {"aether.entity.create", {{"entityId", createdId.ToString()}, {"name", "Transient"}}},
        {"aether.entity.reparent", {{"entityId", createdId.ToString()}, {"parentId", PersistentEntityId::Create().ToString()}}}
    });
    Apply(editorWorld);
    assert(Result(editorWorld, invalidBatch).state == EditRequestState::Rejected);
    assert(editorWorld.GetComponent<WorldDocumentComponent>(documentEntity).authoringWorld->Entities().size() == 2);
    assert(editorWorld.GetComponent<HistoryComponent>(documentEntity).entries.empty());

    const auto acceptedId = PersistentEntityId::Create();
    const auto createAndRename = SubmitEditCommand(editorWorld, documentId, worldInstance, {
        {"aether.entity.create", {{"entityId", acceptedId.ToString()}, {"name", "Before"}}},
        {"aether.entity.rename", {{"entityId", acceptedId.ToString()}, {"name", "After"}}}
    });
    Apply(editorWorld);
    assert(Result(editorWorld, createAndRename).state == EditRequestState::Applied);
    assert(editorWorld.GetComponent<HistoryComponent>(documentEntity).IsDirty());
    auto* authored = editorWorld.GetComponent<WorldDocumentComponent>(documentEntity).authoringWorld.get();
    assert(authored->Entities().size() == 3);
    auto currentEntities = authored->Entities();
    auto newEntity = std::find_if(currentEntities.begin(), currentEntities.end(), [&](EntityId entity) {
        return authored->HasComponent<PersistentEntityIdComponent>(entity)
            && authored->GetComponent<PersistentEntityIdComponent>(entity).value == acceptedId;
    });
    assert(newEntity != currentEntities.end());
    assert(authored->GetComponent<EntityNameComponent>(*newEntity).value == "After");

    const auto undo = SubmitUndo(editorWorld, documentId, worldInstance);
    Apply(editorWorld);
    assert(Result(editorWorld, undo).state == EditRequestState::Applied);
    assert(authored->Entities().size() == 2 && !editorWorld.GetComponent<DocumentComponent>(documentEntity).dirty);
    for (const auto entity : authored->Entities())
        if (authored->HasComponent<MoveOnlyValue>(entity)) assert(*authored->GetComponent<MoveOnlyValue>(entity).value == 42);

    const auto redo = SubmitRedo(editorWorld, documentId, worldInstance);
    Apply(editorWorld);
    assert(Result(editorWorld, redo).state == EditRequestState::Applied);
    assert(authored->Entities().size() == 3);
    MarkDocumentSaved(editorWorld, documentEntity);
    assert(!editorWorld.GetComponent<DocumentComponent>(documentEntity).dirty);

    const auto newRename = SubmitEditCommand(editorWorld, documentId, worldInstance, {
        {"aether.entity.rename", {{"entityId", acceptedId.ToString()}, {"name", "Changed"}}}
    });
    Apply(editorWorld);
    assert(Result(editorWorld, newRename).state == EditRequestState::Applied);
    assert(editorWorld.GetComponent<DocumentComponent>(documentEntity).dirty);
    const auto undoToSaved = SubmitUndo(editorWorld, documentId, worldInstance);
    Apply(editorWorld);
    assert(Result(editorWorld, undoToSaved).state == EditRequestState::Applied);
    assert(!editorWorld.GetComponent<DocumentComponent>(documentEntity).dirty);
    const auto branch = SubmitEditCommand(editorWorld, documentId, worldInstance, {
        {"aether.entity.rename", {{"entityId", acceptedId.ToString()}, {"name", "Branch"}}}
    });
    Apply(editorWorld);
    assert(Result(editorWorld, branch).state == EditRequestState::Applied);
    assert(editorWorld.GetComponent<DocumentComponent>(documentEntity).dirty);
    assert(!editorWorld.GetComponent<HistoryComponent>(documentEntity).entries.empty());

    const auto stale = SubmitEditCommand(editorWorld, documentId, WorldInstanceId::Create(), {
        {"aether.entity.rename", {{"entityId", rootId.ToString()}, {"name", "Stale"}}}
    });
    Apply(editorWorld);
    assert(Result(editorWorld, stale).state == EditRequestState::Rejected);
    currentEntities = authored->Entities();
    auto currentRoot = std::find_if(currentEntities.begin(), currentEntities.end(), [&](EntityId entity) {
        return authored->HasComponent<PersistentEntityIdComponent>(entity)
            && authored->GetComponent<PersistentEntityIdComponent>(entity).value == rootId;
    });
    assert(currentRoot != currentEntities.end());
    assert(authored->GetComponent<EntityNameComponent>(*currentRoot).value == "Root");

    const auto rejectedDelete = SubmitEditCommand(editorWorld, documentId, worldInstance, {
        {"aether.entity.delete-subtree", {{"entityId", rootId.ToString()}}}
    });
    Apply(editorWorld);
    assert(Result(editorWorld, rejectedDelete).state == EditRequestState::Rejected);
    assert(authored->Entities().size() == 3);

    const auto playSession = editorWorld.CreateEntity();
    PlaySessionComponent play;
    play.state = PlayState::Paused;
    editorWorld.AddComponent<PlaySessionComponent>(playSession, std::move(play));
    const auto whilePlaying = SubmitEditCommand(editorWorld, documentId, worldInstance, {
        {"aether.entity.rename", {{"entityId", rootId.ToString()}, {"name", "Locked"}}}
    });
    Apply(editorWorld);
    assert(Result(editorWorld, whilePlaying).state == EditRequestState::Rejected);
    currentEntities = authored->Entities();
    currentRoot = std::find_if(currentEntities.begin(), currentEntities.end(), [&](EntityId entity) {
        return authored->HasComponent<PersistentEntityIdComponent>(entity)
            && authored->GetComponent<PersistentEntityIdComponent>(entity).value == rootId;
    });
    assert(currentRoot != currentEntities.end());
    assert(authored->GetComponent<EntityNameComponent>(*currentRoot).value == "Root");

    editorWorld.DestroyEntity(playSession);
    const auto historySize = editorWorld.GetComponent<HistoryComponent>(documentEntity).entries.size();
    const auto mergeOne = SubmitEditCommand(editorWorld, documentId, worldInstance, {
        {"aether.entity.rename", {{"entityId", acceptedId.ToString()}, {"name", "Merged One"}}}
    });
    Apply(editorWorld);
    assert(Result(editorWorld, mergeOne).state == EditRequestState::Applied);
    const auto afterFirstMerge = editorWorld.GetComponent<HistoryComponent>(documentEntity).entries.size();
    const auto mergeTwo = SubmitEditCommand(editorWorld, documentId, worldInstance, {
        {"aether.entity.rename", {{"entityId", acceptedId.ToString()}, {"name", "Merged Two"}}}
    });
    Apply(editorWorld);
    assert(Result(editorWorld, mergeTwo).state == EditRequestState::Applied);
    assert(afterFirstMerge == historySize);
    assert(editorWorld.GetComponent<HistoryComponent>(documentEntity).entries.size() == afterFirstMerge);
    const auto mergedUndo = SubmitUndo(editorWorld, documentId, worldInstance);
    Apply(editorWorld);
    assert(Result(editorWorld, mergedUndo).state == EditRequestState::Applied);
    assert(authored->GetComponent<EntityNameComponent>(FindById(*authored, acceptedId)).value == "After");

    MarkDocumentSaved(editorWorld, documentEntity);
    const auto beforeCancel = authored->GetComponent<EntityNameComponent>(FindById(*authored, rootId)).value;
    const auto cancelled = SubmitEditCommand(editorWorld, documentId, worldInstance, {
        {"aether.entity.rename", {{"entityId", rootId.ToString()}, {"name", "Cancelled"}}}
    });
    CancelEditCommand(editorWorld, cancelled);
    Apply(editorWorld);
    assert(Result(editorWorld, cancelled).state == EditRequestState::Cancelled);
    assert(authored->GetComponent<EntityNameComponent>(FindById(*authored, rootId)).value == beforeCancel);

    editorWorld.GetComponent<HistoryComponent>(documentEntity).maxBytes = 1;
    const auto overBudget = SubmitEditCommand(editorWorld, documentId, worldInstance, {
        {"aether.entity.rename", {{"entityId", rootId.ToString()}, {"name", "Over Budget"}}}
    });
    Apply(editorWorld);
    assert(Result(editorWorld, overBudget).state == EditRequestState::Applied);
    assert(editorWorld.GetComponent<HistoryComponent>(documentEntity).entries.empty());
    assert(editorWorld.GetComponent<HistoryComponent>(documentEntity).IsDirty());
}
