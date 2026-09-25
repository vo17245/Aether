#include <SampleSceneFeature/EditorRegistration.h>
#include <SampleSceneFeature/Types.h>

#include <cmath>
#include <span>

namespace
{
struct SceneSettingsDraft
{
    std::string displayPath;
    float exposure = 1.0f;
    bool showGrid = true;
};

std::vector<std::byte> Bytes(const std::string& text)
{
    const auto data = std::as_bytes(std::span(text.data(), text.size()));
    return {data.begin(), data.end()};
}

Aether::EditorFramework::FieldDescriptor FloatField(std::string id, std::string label,
    bool (*has)(const Aether::World&, Aether::EntityId), float (*get)(const Aether::World&, Aether::EntityId),
    void (*set)(Aether::World&, Aether::EntityId, float), double minimum, double maximum)
{
    using namespace Aether::EditorFramework;
    FieldDescriptor field;
    field.id = std::move(id);
    field.label = std::move(label);
    field.kind = PropertyKind::Float;
    field.minimum = minimum;
    field.maximum = maximum;
    field.read = [has, get](const Aether::World& world, Aether::EntityId entity) -> std::expected<PropertyValue, std::string> {
        if (!has(world, entity)) return std::unexpected("component is missing");
        return static_cast<double>(get(world, entity));
    };
    field.write = [has, get, set](Aether::World& world, Aether::EntityId entity, const PropertyValue& value) -> std::expected<PropertyValue, std::string> {
        if (!has(world, entity) || !std::holds_alternative<double>(value)) return std::unexpected("field value is invalid");
        const auto previous = static_cast<double>(get(world, entity));
        set(world, entity, static_cast<float>(std::get<double>(value)));
        return previous;
    };
    field.encode = [](const PropertyValue& value) { return Aether::Json(std::get<double>(value)); };
    field.decode = [](const Aether::Json& json) -> std::expected<PropertyValue, std::string> {
        if (!json.is_number()) return std::unexpected("float field expects a number");
        const auto value = json.get<double>();
        if (!std::isfinite(value)) return std::unexpected("float field must be finite");
        return value;
    };
    return field;
}

bool HasMotion(const Aether::World& world, Aether::EntityId entity)
{ return world.HasComponent<SampleSceneFeature::Motion>(entity); }
float GetMotionSpeed(const Aether::World& world, Aether::EntityId entity)
{ return world.GetComponent<SampleSceneFeature::Motion>(entity).speed; }
void SetMotionSpeed(Aether::World& world, Aether::EntityId entity, float value)
{ world.GetComponent<SampleSceneFeature::Motion>(entity).speed = value; }
bool HasSceneSettings(const Aether::World& world, Aether::EntityId entity)
{ return world.HasComponent<SampleSceneFeature::SceneSettings>(entity); }
float GetExposure(const Aether::World& world, Aether::EntityId entity)
{ return world.GetComponent<SampleSceneFeature::SceneSettings>(entity).exposure; }
void SetExposure(Aether::World& world, Aether::EntityId entity, float value)
{ world.GetComponent<SampleSceneFeature::SceneSettings>(entity).exposure = value; }
}

namespace SampleSceneFeature
{
Aether::EditorFramework::EditorFeatureRegistrar::Result RegisterEditor(
    Aether::EditorFramework::EditorFeatureRegistrar& registrar,
    const Aether::GameFeatures::RuntimeRegistry& runtime)
{
    using namespace Aether;
    using namespace EditorFramework;

    ComponentEditorDescriptor motion;
    motion.ownerFeature = "example.sample-scene";
    motion.componentType = "example.motion";
    motion.hasComponent = HasMotion;
    motion.addComponent = [](World& world, EntityId entity) -> std::expected<void, std::string> {
        world.AddComponent<Motion>(entity, Motion{});
        return {};
    };
    motion.removeComponent = [](World& world, EntityId entity) -> std::expected<void, std::string> {
        if (!HasMotion(world, entity)) return std::unexpected("Motion is missing");
        world.RemoveComponent<Motion>(entity);
        return {};
    };
    motion.fields.push_back(FloatField("speed", "Speed", HasMotion, GetMotionSpeed, SetMotionSpeed, 0.0, 10000.0));
    auto registeredMotion = registrar.RegisterComponentEditor(std::move(motion), runtime);
    if (!registeredMotion) return registeredMotion;

    ComponentEditorDescriptor sceneSettings;
    sceneSettings.ownerFeature = "example.sample-scene";
    sceneSettings.componentType = "example.scene-settings-component";
    sceneSettings.hasComponent = HasSceneSettings;
    sceneSettings.addComponent = [](World& world, EntityId entity) -> std::expected<void, std::string> {
        world.AddComponent<SceneSettings>(entity, SceneSettings{});
        return {};
    };
    sceneSettings.removeComponent = [](World& world, EntityId entity) -> std::expected<void, std::string> {
        if (!HasSceneSettings(world, entity)) return std::unexpected("Scene settings are missing");
        world.RemoveComponent<SceneSettings>(entity);
        return {};
    };
    sceneSettings.fields.push_back(FloatField("exposure", "Exposure", HasSceneSettings, GetExposure, SetExposure, 0.0, 64.0));
    FieldDescriptor grid;
    grid.id = "show-grid";
    grid.label = "Show Grid";
    grid.kind = PropertyKind::Boolean;
    grid.read = [](const World& world, EntityId entity) -> std::expected<PropertyValue, std::string> {
        if (!HasSceneSettings(world, entity)) return std::unexpected("component is missing");
        return world.GetComponent<SceneSettings>(entity).showGrid;
    };
    grid.write = [](World& world, EntityId entity, const PropertyValue& value) -> std::expected<PropertyValue, std::string> {
        if (!HasSceneSettings(world, entity) || !std::holds_alternative<bool>(value)) return std::unexpected("field value is invalid");
        auto& current = world.GetComponent<SceneSettings>(entity).showGrid;
        const bool previous = current;
        current = std::get<bool>(value);
        return previous;
    };
    grid.encode = [](const PropertyValue& value) { return Json(std::get<bool>(value)); };
    grid.decode = [](const Json& json) -> std::expected<PropertyValue, std::string> {
        if (!json.is_boolean()) return std::unexpected("boolean field expects true or false");
        return json.get<bool>();
    };
    sceneSettings.fields.push_back(std::move(grid));
    auto registeredSettings = registrar.RegisterComponentEditor(std::move(sceneSettings), runtime);
    if (!registeredSettings) return registeredSettings;

    AssetEditorDescriptor editor;
    editor.ownerFeature = "example.sample-scene";
    editor.supportedTypes = {"example.scene-settings"};
    editor.editorId = "example.scene-settings-editor";
    editor.isDefault = true;
    editor.createDocument = [](World& world, EntityId entity, const ProjectAssets::AssetRecord& record)
        -> std::expected<void, std::string> {
        world.AddComponent<SceneSettingsDraft>(entity, SceneSettingsDraft{record.displayPath});
        return {};
    };
    editor.prepareSave = [](const World& world, EntityId entity)
        -> std::expected<AssetImportProduct, std::string> {
        if (!world.HasComponent<SceneSettingsDraft>(entity)) return std::unexpected("scene settings draft is missing");
        const auto& draft = world.GetComponent<SceneSettingsDraft>(entity);
        const auto text = Json{{"exposure", draft.exposure}, {"showGrid", draft.showGrid}}.dump(2);
        AssetImportProduct product;
        product.displayPath = draft.displayPath;
        product.entryPoint = "Artifacts/scene-settings.json";
        product.sources.push_back({"Source/scene-settings.json", Bytes(text)});
        product.artifacts.push_back({product.entryPoint, Bytes(text)});
        return product;
    };
    return registrar.RegisterAssetEditor(std::move(editor), runtime);
}
} // namespace SampleSceneFeature
