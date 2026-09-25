#include "RuntimeRegistration.h"

#include <PaletteFeature/EditorRegistration.h>
#include <PaletteFeature/Types.h>
#include <SampleSceneFeature/EditorRegistration.h>
#include <SampleSceneFeature/Types.h>
#include <EditorFramework/AssetEditor.h>
#include <EditorFramework/AssetImport.h>
#include <EditorFramework/Commands.h>
#include <EditorFramework/EditorHost.h>
#include <EditorFramework/EditorFeatureRegistrar.h>
#include <EditorFramework/PlayLifecycle.h>
#include <EditorFramework/ProjectLifecycle.h>
#include <EditorFramework/WorldDocument.h>
#include <EditorImGui/CorePanels.h>
#include <EditorImGui/EditorLayer.h>
#include <Entry/Application.h>
#include <GameFeature/EntityMetadata.h>

#include <filesystem>
#include <stdexcept>
#include <utility>

namespace
{
using namespace Aether;
using namespace Aether::EditorFramework;

struct QueuedIntent
{
    std::string type;
    Json payload;
};

std::optional<std::string> ReadString(const Json& json, const char* key)
{
    if (!json.is_object() || !json.contains(key) || !json[key].is_string()) return std::nullopt;
    return json[key].get<std::string>();
}

std::optional<std::uint64_t> ReadUnsigned(const Json& json, const char* key)
{
    if (!json.is_object() || !json.contains(key)) return std::nullopt;
    const auto& value = json[key];
    if (value.is_number_unsigned()) return value.get<std::uint64_t>();
    if (value.is_number_integer() && value.get<std::int64_t>() >= 0)
        return static_cast<std::uint64_t>(value.get<std::int64_t>());
    return std::nullopt;
}

class SampleUiIntentSink final : public EditorImGui::UiIntentSink
{
public:
    void Submit(std::string type, Json payload) override
    {
        m_Queued.push_back({std::move(type), std::move(payload)});
    }

    void Process(EditorHost& host)
    {
        auto queued = std::exchange(m_Queued, {});
        for (const auto& intent : queued) ProcessOne(host, intent);
        PollAssetSaves(host);
    }

private:
    struct PendingAssetSave
    {
        EntityId document = entt::null;
        AssetImportTask task;
    };

    EditorServicesComponent* Services(EditorHost& host)
    {
        if (!host.GetWorld().IsValid(host.SessionEntity())) return nullptr;
        auto& world = host.GetWorld();
        return world.HasComponent<EditorServicesComponent>(host.SessionEntity())
            ? &world.GetComponent<EditorServicesComponent>(host.SessionEntity()) : nullptr;
    }

    ProjectStateComponent* Project(EditorHost& host)
    {
        auto& world = host.GetWorld();
        return world.HasComponent<ProjectStateComponent>(host.SessionEntity())
            ? &world.GetComponent<ProjectStateComponent>(host.SessionEntity()) : nullptr;
    }

    EditorStatusComponent* Status(EditorHost& host)
    {
        auto& world = host.GetWorld();
        return world.HasComponent<EditorStatusComponent>(host.SessionEntity())
            ? &world.GetComponent<EditorStatusComponent>(host.SessionEntity()) : nullptr;
    }

    std::optional<EntityId> FindDocument(EditorHost& host, const GameFeatures::DocumentId& id)
    {
        auto& world = host.GetWorld();
        for (const auto entity : world.Select<DocumentComponent>())
            if (world.GetComponent<DocumentComponent>(entity).id == id) return entity;
        return std::nullopt;
    }

    bool SaveDocument(EditorHost& host, EntityId entity)
    {
        auto& world = host.GetWorld();
        if (!world.IsValid(entity) || !world.HasComponent<DocumentComponent>(entity)) return false;
        auto& document = world.GetComponent<DocumentComponent>(entity);
        if (document.kind == DocumentKind::Asset)
        {
            auto* services = Services(host);
            auto* project = Project(host);
            if (!services || !project || !services->runtime || !services->assetEditors || !services->catalog)
            {
                document.error = "Asset editor services are unavailable.";
                return false;
            }
            auto task = PrepareAssetDocumentSave(world, entity, *services->assetEditors, *services->runtime,
                *services->catalog, project->sessionGeneration, project->projectRoot);
            if (!task) { document.error = task.error(); return false; }
            m_AssetSaves.push_back({entity, std::move(*task)});
            document.error.clear();
            return true;
        }

        auto* services = Services(host);
        auto* project = Project(host);
        if (!services || !project || !services->runtime || !services->catalog ||
            !world.HasComponent<WorldDocumentComponent>(entity))
        {
            document.error = "World document services are unavailable.";
            return false;
        }
        const auto& worldDocument = world.GetComponent<WorldDocumentComponent>(entity);
        if (!worldDocument.authoringWorld) { document.error = "World document is closed."; return false; }
        auto saved = SaveProjectWorldFile(*worldDocument.authoringWorld, document.id,
            project->projectRoot, *services->runtime, *services->catalog);
        if (!saved) { document.error = saved.error(); return false; }
        ++world.GetComponent<WorldDocumentComponent>(entity).revision;
        MarkDocumentSaved(world, entity);
        document.error.clear();
        return true;
    }

    void PollAssetSaves(EditorHost& host)
    {
        auto* services = Services(host);
        auto* project = Project(host);
        if (!services || !project || !services->runtime || !services->catalog) return;
        auto& world = host.GetWorld();
        for (auto iterator = m_AssetSaves.begin(); iterator != m_AssetSaves.end();)
        {
            if (iterator->task.result.wait_for(std::chrono::seconds(0)) != std::future_status::ready)
            { ++iterator; continue; }
            auto prepared = iterator->task.result.get();
            const EntityId entity = iterator->document;
            iterator = m_AssetSaves.erase(iterator);
            if (!prepared)
            {
                if (world.IsValid(entity) && world.HasComponent<DocumentComponent>(entity))
                    world.GetComponent<DocumentComponent>(entity).error = prepared.error();
                continue;
            }
            bool playLocked = false;
            for (const auto playEntity : world.Select<PlaySessionComponent>())
                if (world.GetComponent<PlaySessionComponent>(playEntity).state != PlayState::Stopped) playLocked = true;
            AssetImportCommitContext context{project->projectId, project->sessionGeneration, playLocked,
                project->writable && static_cast<bool>(project->writeLock)};
            auto committed = CommitAssetDocumentSave(world, entity, std::move(*prepared), context,
                *services->catalog, *services->runtime, project->projectRoot);
            if (!committed)
            {
                if (world.IsValid(entity) && world.HasComponent<DocumentComponent>(entity))
                    world.GetComponent<DocumentComponent>(entity).error = committed.error().message;
                continue;
            }
            project->catalog = *committed;
            services->catalog = *committed;
            if (world.IsValid(entity) && world.HasComponent<DocumentComponent>(entity))
                world.GetComponent<DocumentComponent>(entity).error.clear();
        }
    }

    bool CreateWorldDocument(EditorHost& host)
    {
        auto* services = Services(host);
        auto* project = Project(host);
        if (!services || !project || !services->runtime || !services->catalog || !project->open) return false;
        auto& editorWorld = host.GetWorld();
        auto authoringWorld = std::make_unique<World>();
        const auto instanceId = GameFeatures::WorldInstanceId::Create();
        auto mountScope = std::make_unique<GameFeatures::FeatureMountScope>();
        GameFeatures::WorldMountContext mountContext{GameFeatures::WorldRole::Authoring, instanceId, services->catalog};
        auto mounted = mountScope->Mount(*authoringWorld, mountContext, services->runtime->Systems());
        if (!mounted) return false;

        const auto entity = authoringWorld->CreateEntity();
        const auto persistentId = GameFeatures::PersistentEntityId::Create();
        authoringWorld->AddComponent<GameFeatures::PersistentEntityIdComponent>(entity,
            GameFeatures::PersistentEntityIdComponent{persistentId});
        authoringWorld->AddComponent<GameFeatures::EntityNameComponent>(entity,
            GameFeatures::EntityNameComponent{"Sample Entity"});
        authoringWorld->AddComponent<SampleSceneFeature::Motion>(entity, SampleSceneFeature::Motion{1.0f});
        authoringWorld->AddComponent<SampleSceneFeature::SceneSettings>(entity,
            SampleSceneFeature::SceneSettings{1.0f, true});
        authoringWorld->AddComponent<PaletteFeature::PaletteReference>(entity, PaletteFeature::PaletteReference{});

        const auto documentId = GameFeatures::DocumentId::Create();
        const auto documentEntity = editorWorld.CreateEntity();
        WorldDocumentComponent worldDocument{std::move(authoringWorld), std::move(mountScope), instanceId, 0};
        editorWorld.AddComponent<DocumentComponent>(documentEntity,
            DocumentComponent{documentId, DocumentKind::World, false, {}});
        editorWorld.AddComponent<HistoryComponent>(documentEntity);
        editorWorld.AddComponent<WorldDocumentComponent>(documentEntity, std::move(worldDocument));
        PlaySessionComponent playSession;
        playSession.catalog = services->catalog;
        editorWorld.AddComponent<PlaySessionComponent>(documentEntity, std::move(playSession));

        SelectionComponent selection{instanceId, persistentId};
        if (editorWorld.HasComponent<SelectionComponent>(host.SessionEntity()))
            editorWorld.GetComponent<SelectionComponent>(host.SessionEntity()) = std::move(selection);
        else editorWorld.AddComponent<SelectionComponent>(host.SessionEntity(), std::move(selection));
        return true;
    }

    void CloseDocument(EditorHost& host, EntityId entity)
    {
        auto& world = host.GetWorld();
        if (!world.IsValid(entity)) return;
        if (world.HasComponent<WorldDocumentComponent>(entity))
        {
            auto& document = world.GetComponent<WorldDocumentComponent>(entity);
            if (document.mountScope) document.mountScope->Shutdown();
            if (world.HasComponent<PlaySessionComponent>(entity))
                StopPlay(world.GetComponent<PlaySessionComponent>(entity));
        }
        if (world.HasComponent<AssetDocumentComponent>(entity))
            for (auto& save : m_AssetSaves)
                if (save.document == entity) save.task.Cancel();
        if (world.HasComponent<WorldDocumentComponent>(entity) &&
            world.HasComponent<SelectionComponent>(host.SessionEntity()))
        {
            auto& selection = world.GetComponent<SelectionComponent>(host.SessionEntity());
            if (selection.world == world.GetComponent<WorldDocumentComponent>(entity).instanceId)
                selection.entity.reset();
        }
        world.DestroyEntity(entity);
    }

    void StartOrControlPlay(EditorHost& host, std::string_view action)
    {
        auto& world = host.GetWorld();
        for (const auto entity : world.Select<DocumentComponent, WorldDocumentComponent, PlaySessionComponent>())
        {
            const auto& document = world.GetComponent<DocumentComponent>(entity);
            auto& worldDocument = world.GetComponent<WorldDocumentComponent>(entity);
            auto& session = world.GetComponent<PlaySessionComponent>(entity);
            if (action == "start")
            {
                auto* services = Services(host);
                auto* project = Project(host);
                if (!services || !project || !services->runtime || !services->catalog) return;
                bool dirtyAssets = false;
                bool importsActive = false;
                for (const auto open : world.Select<DocumentComponent>())
                    if (world.GetComponent<DocumentComponent>(open).kind == DocumentKind::Asset &&
                        world.GetComponent<DocumentComponent>(open).dirty) dirtyAssets = true;
                for (const auto import : world.Select<AssetImportResultComponent>())
                {
                    const auto state = world.GetComponent<AssetImportResultComponent>(import).state;
                    if (state == AssetImportTaskState::Queued || state == AssetImportTaskState::Running) importsActive = true;
                }
                auto result = StartPlay(session, *worldDocument.authoringWorld, document.id,
                    *services->runtime, services->catalog, project->projectRoot,
                    {.dirtyAssetDocuments = dirtyAssets, .importCommitPending = importsActive});
                if (!result) session.error = result.error().message;
                return;
            }
            if (action == "pause") (void)PausePlay(session);
            else if (action == "resume") (void)ResumePlay(session);
            else if (action == "step") (void)StepPlay(session);
            else if (action == "stop") StopPlay(session);
        }
    }

    void ProcessOne(EditorHost& host, const QueuedIntent& intent)
    {
        auto& editorWorld = host.GetWorld();
        if (intent.type == "editor.document.new-world") { (void)CreateWorldDocument(host); return; }
        if (intent.type == "editor.document.open-world")
        {
            auto* services = Services(host);
            auto* project = Project(host);
            auto* status = Status(host);
            auto path = ReadString(intent.payload, "path");
            if (!services || !project || !services->runtime || !services->catalog || !project->open || !path || path->empty())
                return;
            auto opened = OpenProjectWorldDocument(editorWorld, *path, project->projectRoot,
                services->runtime, services->catalog);
            if (!opened)
            {
                if (status) { status->message = opened.error(); status->isError = true; }
                return;
            }
            if (status) { status->message = "World opened."; status->isError = false; }
            return;
        }
        if (intent.type == "editor.edit.apply")
        {
            auto documentText = ReadString(intent.payload, "documentId");
            auto instanceText = ReadString(intent.payload, "worldInstanceId");
            if (!documentText || !instanceText || !intent.payload.contains("operations") || !intent.payload["operations"].is_array()) return;
            auto document = GameFeatures::DocumentId::Parse(*documentText);
            auto instance = GameFeatures::WorldInstanceId::Parse(*instanceText);
            if (!document || !instance) return;
            std::vector<EditOperation> operations;
            for (const auto& operation : intent.payload["operations"])
            {
                if (!operation.is_object() || !operation.contains("commandId") || !operation["commandId"].is_string() ||
                    !operation.contains("payload")) return;
                operations.push_back({operation["commandId"].get<std::string>(), operation["payload"]});
            }
            (void)SubmitEditCommand(editorWorld, *document, *instance, std::move(operations));
            return;
        }
        if (intent.type == "editor.selection.set")
        {
            auto instanceText = ReadString(intent.payload, "worldInstanceId");
            auto entityText = ReadString(intent.payload, "entityId");
            if (!instanceText || !entityText) return;
            auto instance = GameFeatures::WorldInstanceId::Parse(*instanceText);
            auto entity = GameFeatures::PersistentEntityId::Parse(*entityText);
            if (!instance || !entity) return;
            auto& selection = editorWorld.GetComponent<SelectionComponent>(host.SessionEntity());
            selection.world = *instance;
            selection.entity = *entity;
            return;
        }
        if (intent.type == "editor.history.undo" || intent.type == "editor.history.redo")
        {
            auto documentText = ReadString(intent.payload, "documentId");
            if (!documentText) return;
            auto documentId = GameFeatures::DocumentId::Parse(*documentText);
            if (!documentId) return;
            for (const auto entity : editorWorld.Select<DocumentComponent, WorldDocumentComponent>())
            {
                if (editorWorld.GetComponent<DocumentComponent>(entity).id != *documentId) continue;
                const auto& instance = editorWorld.GetComponent<WorldDocumentComponent>(entity).instanceId;
                if (intent.type == "editor.history.undo") (void)SubmitUndo(editorWorld, *documentId, instance);
                else (void)SubmitRedo(editorWorld, *documentId, instance);
            }
            return;
        }
        if (intent.type == "editor.document.save" || intent.type == "editor.document.close")
        {
            auto documentText = ReadString(intent.payload, "documentId");
            if (!documentText) return;
            auto documentId = GameFeatures::DocumentId::Parse(*documentText);
            if (!documentId) return;
            auto entity = FindDocument(host, *documentId);
            if (!entity) return;
            const bool shouldClose = intent.type == "editor.document.close";
            const auto resolution = ReadString(intent.payload, "resolution").value_or("cancel");
            if (shouldClose && resolution == "cancel") return;
            if (shouldClose && resolution == "save" && !SaveDocument(host, *entity)) return;
            else if (!shouldClose && !SaveDocument(host, *entity)) return;
            if (shouldClose) CloseDocument(host, *entity);
            return;
        }
        if (intent.type == "editor.asset.open")
        {
            auto* services = Services(host);
            auto projectText = ReadString(intent.payload, "projectId");
            auto assetText = ReadString(intent.payload, "assetId");
            if (!services || !services->catalog || !services->assetEditors || !projectText || !assetText) return;
            auto project = ProjectAssets::ProjectId::Parse(*projectText);
            auto asset = ProjectAssets::AssetId::Parse(*assetText);
            if (project && asset)
                (void)OpenAssetDocument(editorWorld, *services->catalog, *services->assetEditors, {*project, *asset});
            return;
        }
        if (intent.type == "editor.asset.rename")
        {
            auto* services = Services(host);
            auto* project = Project(host);
            auto* status = Status(host);
            auto projectText = ReadString(intent.payload, "projectId");
            auto assetText = ReadString(intent.payload, "assetId");
            auto displayPath = ReadString(intent.payload, "displayPath");
            auto sessionGeneration = ReadUnsigned(intent.payload, "sessionGeneration");
            auto catalogGeneration = ReadUnsigned(intent.payload, "catalogGeneration");
            if (!services || !project || !services->runtime || !services->catalog || !projectText || !assetText
                || !displayPath || !sessionGeneration || !catalogGeneration)
                return;
            auto projectId = ProjectAssets::ProjectId::Parse(*projectText);
            auto assetId = ProjectAssets::AssetId::Parse(*assetText);
            if (!projectId || !assetId) return;
            bool playLocked = false;
            for (const auto entity : editorWorld.Select<PlaySessionComponent>())
                if (editorWorld.GetComponent<PlaySessionComponent>(entity).state != PlayState::Stopped)
                    playLocked = true;
            AssetImportCommitContext context{project->projectId, project->sessionGeneration, playLocked,
                project->writable && static_cast<bool>(project->writeLock)};
            auto renamed = CommitAssetRename({*projectId, *sessionGeneration, *catalogGeneration,
                    *assetId, *displayPath}, context, *services->catalog, *services->runtime, project->projectRoot);
            if (!renamed)
            {
                if (status) { status->message = renamed.error().message; status->isError = true; }
                return;
            }
            project->catalog = *renamed;
            services->catalog = *renamed;
            if (status) { status->message = "Asset renamed."; status->isError = false; }
            return;
        }
        if (intent.type == "editor.asset.import")
        {
            auto* services = Services(host);
            auto* project = Project(host);
            auto path = ReadString(intent.payload, "path");
            auto importer = ReadString(intent.payload, "importerId");
            if (!services || !project || !services->catalog || !path || path->empty() || !importer || importer->empty()) return;
            AssetImportRequest request;
            request.projectId = services->catalog->Project();
            request.sessionGeneration = project->sessionGeneration;
            request.catalogGeneration = services->catalog->Generation();
            request.importerId = *importer;
            request.inputs.push_back({std::filesystem::path(*path).filename().generic_string(), *path});
            request.settings = Json::object();
            (void)SubmitAssetImport(editorWorld, std::move(request));
            return;
        }
        if (intent.type.starts_with("editor.play."))
        {
            StartOrControlPlay(host, intent.type.substr(std::string("editor.play.").size()));
            return;
        }
    }

    std::vector<QueuedIntent> m_Queued;
    std::vector<PendingAssetSave> m_AssetSaves;
};

class SampleEditorApplication final : public Application
{
public:
    void OnInit(Window& window) override
    {
        auto runtime = SampleGame::BuildRuntimeRegistry(SampleGame::MakeProjectManifest());
        if (!runtime) throw std::runtime_error(runtime.error().message);

        EditorFeatureRegistrar paletteEditor(*(*runtime)->FindFeature("example.palette"));
        EditorFeatureRegistrar sceneEditor(*(*runtime)->FindFeature("example.sample-scene"));
        auto paletteRegistered = PaletteFeature::RegisterEditor(paletteEditor, **runtime);
        auto sceneRegistered = SampleSceneFeature::RegisterEditor(sceneEditor, **runtime);
        if (!paletteRegistered || !sceneRegistered)
            throw std::runtime_error(!paletteRegistered ? paletteRegistered.error() : sceneRegistered.error());
        paletteEditor.Freeze();
        sceneEditor.Freeze();

        auto schemas = std::make_shared<ComponentSchemaRegistry>();
        for (const auto& [id, descriptor] : paletteEditor.ComponentEditors())
        { (void)id; auto result = schemas->Register(descriptor, **runtime); if (!result) throw std::runtime_error(result.error()); }
        for (const auto& [id, descriptor] : sceneEditor.ComponentEditors())
        { (void)id; auto result = schemas->Register(descriptor, **runtime); if (!result) throw std::runtime_error(result.error()); }
        schemas->Freeze();

        auto assetEditors = std::make_shared<AssetEditorRegistry>();
        for (const auto& [id, descriptor] : paletteEditor.AssetEditors())
        { (void)id; auto result = assetEditors->Register(descriptor, **runtime); if (!result) throw std::runtime_error(result.error()); }
        for (const auto& [id, descriptor] : sceneEditor.AssetEditors())
        { (void)id; auto result = assetEditors->Register(descriptor, **runtime); if (!result) throw std::runtime_error(result.error()); }
        assetEditors->Freeze();

        auto importers = std::make_shared<AssetImporterRegistry>();
        auto paletteImporter = PaletteFeature::RegisterImporter(*importers, **runtime);
        auto sceneImporter = SampleSceneFeature::RegisterImporter(*importers, **runtime);
        if (!paletteImporter || !sceneImporter)
            throw std::runtime_error(!paletteImporter ? paletteImporter.error() : sceneImporter.error());
        importers->Freeze();

        auto commands = std::make_shared<CommandRegistry>();
        auto core = RegisterCoreEntityCommands(*commands, *runtime);
        auto inspector = RegisterInspectorCommands(*commands, schemas, *runtime);
        if (!core || !inspector) throw std::runtime_error(!core ? core.error() : inspector.error());
        commands->Freeze();

        m_Host = std::make_unique<EditorHost>();
        auto& editorWorld = m_Host->GetWorld();
        auto& services = editorWorld.AddComponent<EditorServicesComponent>(m_Host->SessionEntity(),
            EditorServicesComponent{.commands = commands, .runtime = *runtime,
                .componentSchemas = schemas, .assetEditors = assetEditors});
        editorWorld.AddComponent<EditorStatusComponent>(m_Host->SessionEntity());
        auto& project = editorWorld.GetComponent<ProjectStateComponent>(m_Host->SessionEntity());
        auto opened = OpenProjectContext(project, services, std::filesystem::current_path() / "TestProject",
            *runtime, importers, true);
        if (!opened) throw std::runtime_error(opened.error());

        CreateWorldDocument(*m_Host);
        if (!EditorImGui::RegisterCorePanels(m_Panels))
            throw std::runtime_error("could not compose the standard editor panels");
        EditorImGui::EditorLayerCallbacks callbacks;
        callbacks.onUpdate = [this](float) { m_Intents.Process(*m_Host); };
        m_Layer = std::make_unique<EditorImGui::EditorLayer>(*m_Host, m_Panels, m_Intents,
            std::move(callbacks));
        window.PushLayer(m_Layer.get());
    }

    void OnShutdown() override
    {
        if (m_Host)
        {
            auto& world = m_Host->GetWorld();
            for (const auto entity : world.Select<PlaySessionComponent>())
                StopPlay(world.GetComponent<PlaySessionComponent>(entity));
            if (world.IsValid(m_Host->SessionEntity()))
            {
                if (world.HasComponent<ProjectStateComponent>(m_Host->SessionEntity()))
                {
                    auto& project = world.GetComponent<ProjectStateComponent>(m_Host->SessionEntity());
                    auto& services = world.GetComponent<EditorServicesComponent>(m_Host->SessionEntity());
                    (void)CloseProjectContext(project, services, {.dirtyDocuments = false,
                        .discardDirtyDocuments = true, .playActive = false, .importTasksActive = false});
                }
            }
        }
    }

    const char* GetName() const override { return "Sample Game Editor"; }

private:
    static bool CreateWorldDocument(EditorHost& host)
    {
        auto& editorWorld = host.GetWorld();
        auto& services = editorWorld.GetComponent<EditorServicesComponent>(host.SessionEntity());
        auto& project = editorWorld.GetComponent<ProjectStateComponent>(host.SessionEntity());
        auto authoringWorld = std::make_unique<World>();
        const auto instanceId = GameFeatures::WorldInstanceId::Create();
        auto mountScope = std::make_unique<GameFeatures::FeatureMountScope>();
        GameFeatures::WorldMountContext mountContext{GameFeatures::WorldRole::Authoring, instanceId, services.catalog};
        auto mounted = mountScope->Mount(*authoringWorld, mountContext, services.runtime->Systems());
        if (!mounted) return false;
        const auto entity = authoringWorld->CreateEntity();
        const auto persistentId = GameFeatures::PersistentEntityId::Create();
        authoringWorld->AddComponent<GameFeatures::PersistentEntityIdComponent>(entity,
            GameFeatures::PersistentEntityIdComponent{persistentId});
        authoringWorld->AddComponent<GameFeatures::EntityNameComponent>(entity,
            GameFeatures::EntityNameComponent{"Sample Entity"});
        authoringWorld->AddComponent<SampleSceneFeature::Motion>(entity, SampleSceneFeature::Motion{1.0f});
        authoringWorld->AddComponent<SampleSceneFeature::SceneSettings>(entity,
            SampleSceneFeature::SceneSettings{1.0f, true});
        authoringWorld->AddComponent<PaletteFeature::PaletteReference>(entity, PaletteFeature::PaletteReference{});
        const auto documentId = GameFeatures::DocumentId::Create();
        const auto documentEntity = editorWorld.CreateEntity();
        editorWorld.AddComponent<DocumentComponent>(documentEntity,
            DocumentComponent{documentId, DocumentKind::World, false, {}});
        editorWorld.AddComponent<HistoryComponent>(documentEntity);
        editorWorld.AddComponent<WorldDocumentComponent>(documentEntity,
            WorldDocumentComponent{std::move(authoringWorld), std::move(mountScope), instanceId, 0});
        PlaySessionComponent play;
        play.catalog = services.catalog;
        editorWorld.AddComponent<PlaySessionComponent>(documentEntity, std::move(play));
        editorWorld.AddComponent<SelectionComponent>(host.SessionEntity(), SelectionComponent{instanceId, persistentId});
        (void)project;
        return true;
    }

    std::unique_ptr<EditorHost> m_Host;
    EditorImGui::UiSystemRegistry m_Panels;
    SampleUiIntentSink m_Intents;
    std::unique_ptr<EditorImGui::EditorLayer> m_Layer;
};
}

DEFINE_APPLICATION(SampleEditorApplication)
