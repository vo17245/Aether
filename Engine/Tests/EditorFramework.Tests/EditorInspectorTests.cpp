#include <EditorFramework/ComponentSchema.h>
#include <EditorFramework/EditorFeatureRegistrar.h>
#include <GameFeature/EntityMetadata.h>

#include <algorithm>
#include <cassert>

namespace
{
struct Inspectable
{
    std::string title = "Initial";
    double weight = 1.0;
    std::optional<Aether::ProjectAssets::ProjectAssetRef> texture;
};
}

namespace Aether::Serialization
{
template<> struct ComponentSerialization<Inspectable>
{
    static constexpr std::string_view Type = "sample.inspectable";
    static constexpr std::uint32_t Version = 1;
    static Result<Json> Serialize(const Inspectable& value, SaveContext&)
    {
        Json texture = nullptr;
        if (value.texture) texture = Json{{"projectId", value.texture->project.ToString()}, {"assetId", value.texture->asset.ToString()}};
        return Json{{"title", value.title}, {"weight", value.weight}, {"texture", texture}};
    }
    static Result<Inspectable> Deserialize(const Json& json, std::uint32_t version, LoadContext&)
    {
        if (version != 1 || !json.is_object() || !json.contains("title") || !json["title"].is_string()
            || !json.contains("weight") || !json["weight"].is_number() || !json.contains("texture"))
            return std::unexpected(ArchiveError{ArchiveErrorCode::InvalidFormat, "invalid Inspectable"});
        Inspectable result{json["title"].get<std::string>(), json["weight"].get<double>(), std::nullopt};
        if (!json["texture"].is_null())
        {
            if (!json["texture"].is_object() || !json["texture"].contains("projectId") || !json["texture"].contains("assetId"))
                return std::unexpected(ArchiveError{ArchiveErrorCode::InvalidFormat, "invalid texture ref"});
            auto project = ProjectAssets::ProjectId::Parse(json["texture"]["projectId"].get<std::string>());
            auto asset = ProjectAssets::AssetId::Parse(json["texture"]["assetId"].get<std::string>());
            if (!project || !asset) return std::unexpected(ArchiveError{ArchiveErrorCode::InvalidFormat, "invalid texture IDs"});
            result.texture = ProjectAssets::ProjectAssetRef{*project, *asset};
        }
        return result;
    }
};
}

int main()
{
    using namespace Aether;
    using namespace Aether::GameFeatures;
    using namespace Aether::EditorFramework;
    using namespace Aether::ProjectAssets;

    ProjectManifest manifest;
    manifest.projectId = ProjectId::Create();
    manifest.features = {{"sample.runtime", 1}, {"sample.texture", 1}, {"sample.material", 1}};
    CompiledFeature runtime;
    runtime.descriptor = {"sample.runtime", 1, 1, {}};
    runtime.registerRuntime = [](RuntimeFeatureRegistrar& registrar) {
        auto base = RegisterBaseRuntimeComponents(registrar);
        if (!base) return base;
        auto inspectable = registrar.RegisterComponent<Inspectable>({{"texture", "sample.texture", true}}, {},
            [](const Inspectable& component, const RuntimeFeatureRegistrar::AssetReferenceCallback& visit) {
                visit({"texture", "sample.texture", component.texture});
            });
        if (!inspectable) return inspectable;
        AssetTypeDescriptor metadata;
        metadata.id = "sample.runtime-meta";
        metadata.ownerFeature = registrar.Feature().id;
        auto registered = registrar.RegisterAssetType(std::move(metadata));
        if (!registered) return FeatureResult<void>(std::unexpected(FeatureError{registrar.Feature().id, registered.error().message}));
        return FeatureResult<void>{};
    };
    auto textureFeature = [](std::string featureId, std::string assetType) {
        CompiledFeature feature;
        feature.descriptor = {std::move(featureId), 1, 1, {}};
        feature.registerRuntime = [assetType = std::move(assetType)](RuntimeFeatureRegistrar& registrar) -> FeatureResult<void> {
            AssetTypeDescriptor descriptor;
            descriptor.id = assetType;
            descriptor.ownerFeature = registrar.Feature().id;
            auto registered = registrar.RegisterAssetType(std::move(descriptor));
            if (!registered) return std::unexpected(FeatureError{registrar.Feature().id, registered.error().message});
            return {};
        };
        return feature;
    };
    auto built = RuntimeRegistry::Build(manifest,
        {runtime, textureFeature("sample.texture", "sample.texture"), textureFeature("sample.material", "sample.material")});
    assert(built);
    const auto& registry = **built;

    ComponentEditorDescriptor descriptor;
    descriptor.ownerFeature = "sample.runtime";
    descriptor.componentType = "sample.inspectable";
    descriptor.hasComponent = [](const World& world, EntityId entity) { return world.HasComponent<Inspectable>(entity); };
    descriptor.addComponent = [](World& world, EntityId entity) -> std::expected<void, std::string> {
        world.AddComponent<Inspectable>(entity, Inspectable{});
        return {};
    };
    descriptor.removeComponent = [](World& world, EntityId entity) -> std::expected<void, std::string> {
        if (!world.HasComponent<Inspectable>(entity)) return std::unexpected("Inspectable is missing");
        world.RemoveComponent<Inspectable>(entity);
        return {};
    };
    FieldDescriptor title;
    title.id = "title"; title.label = "Title"; title.kind = PropertyKind::String;
    title.read = [](const World& world, EntityId entity) -> std::expected<PropertyValue, std::string> {
        return PropertyValue{world.GetComponent<Inspectable>(entity).title};
    };
    title.write = [](World& world, EntityId entity, const PropertyValue& value) -> std::expected<PropertyValue, std::string> {
        auto& component = world.GetComponent<Inspectable>(entity);
        PropertyValue old = component.title;
        component.title = std::get<std::string>(value);
        return old;
    };
    FieldDescriptor weight;
    weight.id = "weight"; weight.label = "Weight"; weight.kind = PropertyKind::Float;
    weight.minimum = 0.0; weight.maximum = 10.0;
    weight.read = [](const World& world, EntityId entity) -> std::expected<PropertyValue, std::string> {
        return PropertyValue{world.GetComponent<Inspectable>(entity).weight};
    };
    weight.write = [](World& world, EntityId entity, const PropertyValue& value) -> std::expected<PropertyValue, std::string> {
        auto& component = world.GetComponent<Inspectable>(entity);
        PropertyValue old = component.weight;
        component.weight = std::get<double>(value);
        return old;
    };
    FieldDescriptor textureField;
    textureField.id = "texture"; textureField.label = "Texture"; textureField.kind = PropertyKind::AssetReference;
    textureField.expectedAssetType = "sample.texture"; textureField.nullable = true;
    textureField.read = [](const World& world, EntityId entity) -> std::expected<PropertyValue, std::string> {
        return PropertyValue{EditorFramework::AssetReferenceValue{world.GetComponent<Inspectable>(entity).texture}};
    };
    textureField.write = [](World& world, EntityId entity, const PropertyValue& value) -> std::expected<PropertyValue, std::string> {
        auto& component = world.GetComponent<Inspectable>(entity);
        PropertyValue old = EditorFramework::AssetReferenceValue{component.texture};
        component.texture = std::get<EditorFramework::AssetReferenceValue>(value).value;
        return old;
    };
    descriptor.fields = {title, weight, textureField};
    ComponentSchemaRegistry schemas;
    assert(schemas.Register(descriptor, registry));
    auto invalidSchema = descriptor;
    invalidSchema.ownerFeature = "sample.material";
    assert(!schemas.Register(invalidSchema, registry));

    EditorFeatureRegistrar editorFeature({"sample.runtime", 1, 1, {}});
    assert(editorFeature.RegisterComponentEditor(descriptor, registry));
    assert(!editorFeature.RegisterComponentEditor(descriptor, registry));
    assert(editorFeature.RegisterViewportProvider("sample.viewport"));
    assert(!editorFeature.RegisterViewportProvider("sample.viewport"));
    editorFeature.Freeze();
    assert(!editorFeature.RegisterViewportProvider("sample.late"));
    auto makeAssetEditor = [](std::string owner, std::string type, std::string editorId) {
        AssetEditorDescriptor editor;
        editor.ownerFeature = std::move(owner);
        editor.supportedTypes = {std::move(type)};
        editor.editorId = std::move(editorId);
        editor.isDefault = true;
        editor.createDocument = [](World&, EntityId, const AssetRecord&) -> std::expected<void, std::string> { return {}; };
        editor.prepareSave = [](const World&, EntityId) -> std::expected<AssetImportProduct, std::string> {
            return AssetImportProduct{};
        };
        return editor;
    };
    auto textureDescriptor = makeAssetEditor("sample.texture", "sample.texture", "sample.texture-editor");
    auto materialDescriptor = makeAssetEditor("sample.material", "sample.material", "sample.material-editor");
    EditorFeatureRegistrar textureEditor({"sample.texture", 1, 1, {}});
    assert(textureEditor.RegisterAssetEditor(textureDescriptor, registry));
    assert(!textureEditor.RegisterAssetEditor(makeAssetEditor("sample.texture", "sample.texture", "sample.texture-alt"), registry));
    assert(!textureEditor.RegisterAssetEditor(makeAssetEditor("sample.texture", "sample.material", "sample.bad-owner"), registry));
    AssetEditorRegistry assetEditors;
    assert(assetEditors.Register(textureDescriptor, registry));
    assert(assetEditors.Register(materialDescriptor, registry));
    assert(!assetEditors.Register(makeAssetEditor("sample.texture", "sample.texture", "sample.texture-alt"), registry));

    AssetTypeRegistry assetTypes;
    for (const auto type : {"sample.texture", "sample.material", "sample.runtime-meta"})
    {
        AssetTypeDescriptor assetType;
        assetType.id = type; assetType.ownerFeature = type == std::string_view("sample.runtime-meta") ? "sample.runtime" : type;
        assert(assetTypes.Register(std::move(assetType)));
    }
    assetTypes.Freeze();
    const auto validAsset = AssetId::Create();
    const auto wrongTypeAsset = AssetId::Create();
    const auto metadataAsset = AssetId::Create();
    auto makeRecord = [](AssetId id, std::string type, std::string displayPath) {
        AssetRecord record;
        record.id = id; record.type = type;
        record.ownerFeature = type == "sample.runtime-meta" ? "sample.runtime" : type;
        record.revision = 1;
        record.displayPath = displayPath;
        record.artifactRoot = "Assets/Data/" + id.ToString() + "/1";
        record.entryPoint = "asset.json";
        record.files.push_back({"asset.json", 2});
        return record;
    };
    auto catalogResult = CatalogSnapshot::Create({manifest.projectId, 3, {
        makeRecord(validAsset, "sample.texture", "Textures/Stone"),
        makeRecord(wrongTypeAsset, "sample.material", "Materials/Wrong"),
        makeRecord(metadataAsset, "sample.runtime-meta", "Metadata/ReadOnly")}}, assetTypes);
    assert(catalogResult);

    World world;
    const auto validEntity = world.CreateEntity();
    const auto persistentId = PersistentEntityId::Create();
    world.AddComponent<PersistentEntityIdComponent>(validEntity, PersistentEntityIdComponent{persistentId});
    world.AddComponent<Inspectable>(validEntity, Inspectable{"Stone", 4.0,
        ProjectAssetRef{manifest.projectId, validAsset}});
    const auto missingEntity = world.CreateEntity();
    const auto missingId = PersistentEntityId::Create();
    const auto absentAsset = AssetId::Create();
    world.AddComponent<PersistentEntityIdComponent>(missingEntity, PersistentEntityIdComponent{missingId});
    world.AddComponent<Inspectable>(missingEntity, Inspectable{"Missing", 2.0,
        ProjectAssetRef{manifest.projectId, absentAsset}});
    const auto wrongEntity = world.CreateEntity();
    const auto wrongId = PersistentEntityId::Create();
    world.AddComponent<PersistentEntityIdComponent>(wrongEntity, PersistentEntityIdComponent{wrongId});
    world.AddComponent<Inspectable>(wrongEntity, Inspectable{"Wrong", 3.0,
        ProjectAssetRef{manifest.projectId, wrongTypeAsset}});

    const auto instance = WorldInstanceId::Create();
    auto model = BuildInspectorModel(world, {instance, persistentId}, instance, registry, schemas, **catalogResult);
    assert(model.validTarget && model.components.size() == 2);
    const auto component = std::find_if(model.components.begin(), model.components.end(), [](const auto& item) {
        return item.componentType == "sample.inspectable";
    });
    assert(component != model.components.end() && !component->readOnly && component->fields.size() == 3);
    const auto textureModel = std::find_if(component->fields.begin(), component->fields.end(), [](const auto& item) { return item.fieldId == "texture"; });
    assert(textureModel != component->fields.end());
    assert(textureModel->expectedAssetType == "sample.texture" && textureModel->actualAssetType == "sample.texture");
    assert(textureModel->actualAssetId == validAsset.ToString() && textureModel->actualDisplayPath == "Textures/Stone");

    auto missingModel = BuildInspectorModel(world, {instance, missingId}, instance, registry, schemas, **catalogResult);
    const auto& missingFields = std::find_if(missingModel.components.begin(), missingModel.components.end(), [](const auto& item) {
        return item.componentType == "sample.inspectable";
    })->fields;
    const auto missingTexture = std::find_if(missingFields.begin(), missingFields.end(), [](const auto& item) { return item.fieldId == "texture"; });
    assert(missingTexture->expectedAssetType == "sample.texture" && missingTexture->error == "asset is missing from the project catalog");
    auto wrongModel = BuildInspectorModel(world, {instance, wrongId}, instance, registry, schemas, **catalogResult);
    const auto& wrongFields = std::find_if(wrongModel.components.begin(), wrongModel.components.end(), [](const auto& item) {
        return item.componentType == "sample.inspectable";
    })->fields;
    const auto wrongTexture = std::find_if(wrongFields.begin(), wrongFields.end(), [](const auto& item) { return item.fieldId == "texture"; });
    assert(wrongTexture->expectedAssetType == "sample.texture" && wrongTexture->actualAssetType == "sample.material");
    assert(wrongTexture->error == "asset type does not match field");
    assert(!BuildInspectorModel(world, {WorldInstanceId::Create(), persistentId}, instance, registry, schemas, **catalogResult).validTarget);

    assert(!weight.DecodeValue(20.0));
    assert(textureField.DecodeValue(nullptr));
    assert(!textureField.DecodeValue(Json{{"projectId", ProjectId::Create().ToString()}, {"assetId", "not-a-uuid"}}));

    int textureFactoryCount = 0;
    textureDescriptor.createDocument = [&textureFactoryCount](World&, EntityId, const AssetRecord&) -> std::expected<void, std::string> {
        ++textureFactoryCount;
        return {};
    };
    AssetEditorRegistry openEditors;
    assert(openEditors.Register(textureDescriptor, registry));
    assert(openEditors.Register(materialDescriptor, registry));
    World editorWorld;
    auto openedTexture = OpenAssetDocument(editorWorld, **catalogResult, openEditors, {manifest.projectId, validAsset});
    assert(openedTexture && !openedTexture->reused && !openedTexture->readOnly && textureFactoryCount == 1);
    auto reopenedTexture = OpenAssetDocument(editorWorld, **catalogResult, openEditors, {manifest.projectId, validAsset});
    assert(reopenedTexture && reopenedTexture->reused && reopenedTexture->entity == openedTexture->entity && textureFactoryCount == 1);
    auto openedMaterial = OpenAssetDocument(editorWorld, **catalogResult, openEditors, {manifest.projectId, wrongTypeAsset});
    assert(openedMaterial && !openedMaterial->readOnly);
    auto openedMetadata = OpenAssetDocument(editorWorld, **catalogResult, openEditors, {manifest.projectId, metadataAsset});
    assert(openedMetadata && openedMetadata->readOnly);

    auto commandRegistry = std::make_shared<CommandRegistry>();
    assert(RegisterCoreEntityCommands(*commandRegistry, *built));
    auto sharedSchemas = std::make_shared<ComponentSchemaRegistry>(schemas);
    assert(RegisterInspectorCommands(*commandRegistry, sharedSchemas, *built));
    commandRegistry->Freeze();

    auto editable = std::make_unique<World>();
    const auto editableEntity = editable->CreateEntity();
    const auto editableId = PersistentEntityId::Create();
    editable->AddComponent<PersistentEntityIdComponent>(editableEntity, PersistentEntityIdComponent{editableId});
    World commandWorld;
    const auto serviceEntity = commandWorld.CreateEntity();
    commandWorld.AddComponent<EditorServicesComponent>(serviceEntity, EditorServicesComponent{
        commandRegistry, *built, *catalogResult, std::filesystem::temp_directory_path()});
    const auto documentId = DocumentId::Create();
    const auto instanceId = WorldInstanceId::Create();
    const auto documentEntity = commandWorld.CreateEntity();
    commandWorld.AddComponent<DocumentComponent>(documentEntity, DocumentComponent{documentId, DocumentKind::World, false, {}});
    WorldDocumentComponent worldDocument;
    worldDocument.authoringWorld = std::move(editable);
    worldDocument.instanceId = instanceId;
    commandWorld.AddComponent<WorldDocumentComponent>(documentEntity, std::move(worldDocument));
    commandWorld.AddComponent<HistoryComponent>(documentEntity);

    auto addRequest = SubmitEditCommand(commandWorld, documentId, instanceId, {
        {"aether.component.add", {{"entityId", editableId.ToString()}, {"componentType", "sample.inspectable"}}}});
    ApplyEditCommands(commandWorld);
    auto& authoringWorld = *commandWorld.GetComponent<WorldDocumentComponent>(documentEntity).authoringWorld;
    auto findEditable = [&]() -> EntityId {
        for (const auto entity : authoringWorld.Select<PersistentEntityIdComponent>())
            if (authoringWorld.GetComponent<PersistentEntityIdComponent>(entity).value == editableId) return entity;
        return entt::null;
    };
    EntityId located = findEditable();
    assert(located != entt::null && authoringWorld.HasComponent<Inspectable>(located));
    assert(commandWorld.GetComponent<EditCommandResultComponent>(addRequest).state == EditRequestState::Applied);

    auto undoRequest = SubmitUndo(commandWorld, documentId, instanceId);
    ApplyEditCommands(commandWorld);
    assert(commandWorld.GetComponent<EditCommandResultComponent>(undoRequest).state == EditRequestState::Applied);
    located = findEditable();
    assert(!authoringWorld.HasComponent<Inspectable>(located));

    auto redoRequest = SubmitRedo(commandWorld, documentId, instanceId);
    ApplyEditCommands(commandWorld);
    assert(commandWorld.GetComponent<EditCommandResultComponent>(redoRequest).state == EditRequestState::Applied);
    located = findEditable();
    assert(authoringWorld.HasComponent<Inspectable>(located));

    auto removeRequest = SubmitEditCommand(commandWorld, documentId, instanceId, {
        {"aether.component.remove", {{"entityId", editableId.ToString()}, {"componentType", "sample.inspectable"}}}});
    ApplyEditCommands(commandWorld);
    assert(commandWorld.GetComponent<EditCommandResultComponent>(removeRequest).state == EditRequestState::Applied);
    located = findEditable();
    assert(!authoringWorld.HasComponent<Inspectable>(located));
}
