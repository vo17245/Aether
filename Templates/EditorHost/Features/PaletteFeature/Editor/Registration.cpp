#include <PaletteFeature/EditorRegistration.h>
#include <PaletteFeature/Types.h>

#include <array>

namespace
{
struct PaletteDraft
{
    std::string displayPath;
    std::vector<std::string> colors{"#FFFFFFFF"};
};

std::vector<std::byte> Bytes(const std::string& text)
{
    const auto data = std::as_bytes(std::span(text.data(), text.size()));
    return {data.begin(), data.end()};
}
}

namespace PaletteFeature
{
Aether::EditorFramework::EditorFeatureRegistrar::Result RegisterEditor(
    Aether::EditorFramework::EditorFeatureRegistrar& registrar,
    const Aether::GameFeatures::RuntimeRegistry& runtime)
{
    using namespace Aether;
    using namespace EditorFramework;

    ComponentEditorDescriptor component;
    component.ownerFeature = "example.palette";
    component.componentType = "example.palette-reference";
    component.hasComponent = [](const World& world, EntityId entity) {
        return world.HasComponent<PaletteReference>(entity);
    };
    component.addComponent = [](World& world, EntityId entity) -> std::expected<void, std::string> {
        world.AddComponent<PaletteReference>(entity, PaletteReference{});
        return {};
    };
    component.removeComponent = [](World& world, EntityId entity) -> std::expected<void, std::string> {
        if (!world.HasComponent<PaletteReference>(entity)) return std::unexpected("Palette reference is missing");
        world.RemoveComponent<PaletteReference>(entity);
        return {};
    };
    FieldDescriptor palette;
    palette.id = "palette";
    palette.label = "Palette";
    palette.kind = PropertyKind::AssetReference;
    palette.expectedAssetType = "example.palette";
    palette.nullable = true;
    palette.read = [](const World& world, EntityId entity) -> std::expected<PropertyValue, std::string> {
        return AssetReferenceValue{world.GetComponent<PaletteReference>(entity).value};
    };
    palette.write = [](World& world, EntityId entity, const PropertyValue& value) -> std::expected<PropertyValue, std::string> {
        if (!std::holds_alternative<AssetReferenceValue>(value)) return std::unexpected("Palette field expects an asset reference");
        auto& current = world.GetComponent<PaletteReference>(entity).value;
        AssetReferenceValue previous{current};
        current = std::get<AssetReferenceValue>(value).value;
        return previous;
    };
    palette.encode = [](const PropertyValue& value) {
        const auto& reference = std::get<AssetReferenceValue>(value).value;
        if (!reference) return Json(nullptr);
        return Json{{"projectId", reference->project.ToString()}, {"assetId", reference->asset.ToString()}};
    };
    palette.decode = [](const Json& json) -> std::expected<PropertyValue, std::string> {
        if (json.is_null()) return PropertyValue{AssetReferenceValue{}};
        if (!json.is_object() || !json.contains("projectId") || !json["projectId"].is_string() ||
            !json.contains("assetId") || !json["assetId"].is_string())
            return std::unexpected("asset reference requires projectId and assetId");
        auto project = ProjectAssets::ProjectId::Parse(json["projectId"].get<std::string>());
        auto asset = ProjectAssets::AssetId::Parse(json["assetId"].get<std::string>());
        if (!project || !asset) return std::unexpected("asset reference contains a non-canonical UUID");
        return PropertyValue{AssetReferenceValue{ProjectAssets::ProjectAssetRef{*project, *asset}}};
    };
    component.fields.push_back(std::move(palette));
    auto componentResult = registrar.RegisterComponentEditor(std::move(component), runtime);
    if (!componentResult) return componentResult;

    AssetEditorDescriptor editor;
    editor.ownerFeature = "example.palette";
    editor.supportedTypes = {"example.palette"};
    editor.editorId = "example.palette-editor";
    editor.isDefault = true;
    editor.createDocument = [](World& world, EntityId entity, const ProjectAssets::AssetRecord& record)
        -> std::expected<void, std::string> {
        world.AddComponent<PaletteDraft>(entity, PaletteDraft{record.displayPath});
        return {};
    };
    editor.prepareSave = [](const World& world, EntityId entity)
        -> std::expected<AssetImportProduct, std::string> {
        if (!world.HasComponent<PaletteDraft>(entity)) return std::unexpected("palette draft is missing");
        const auto& draft = world.GetComponent<PaletteDraft>(entity);
        const auto text = Json{{"colors", draft.colors}}.dump(2);
        AssetImportProduct product;
        product.displayPath = draft.displayPath;
        product.entryPoint = "Artifacts/palette.json";
        product.sources.push_back({"Source/palette.json", Bytes(text)});
        product.artifacts.push_back({product.entryPoint, Bytes(text)});
        return product;
    };
    return registrar.RegisterAssetEditor(std::move(editor), runtime);
}
} // namespace PaletteFeature
