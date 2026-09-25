#include <SampleSceneFeature/EditorRegistration.h>

#include <span>

namespace SampleSceneFeature
{
Aether::EditorFramework::EditorFeatureRegistrar::Result RegisterImporter(
    Aether::EditorFramework::AssetImporterRegistry& importers,
    const Aether::GameFeatures::RuntimeRegistry& runtime)
{
    using namespace Aether;
    EditorFramework::AssetImporterDescriptor importer;
    importer.id = "example.scene-settings-importer";
    importer.ownerFeature = "example.sample-scene";
    importer.outputType = "example.scene-settings";
    importer.version = 1;
    importer.extensions = {".json"};
    importer.probe = [](std::span<const std::byte> bytes) { return !bytes.empty(); };
    importer.import = [](const std::vector<EditorFramework::ImportInputData>& inputs, const Json& settings)
        -> std::expected<EditorFramework::AssetImportProduct, std::string> {
        if (inputs.empty()) return std::unexpected("choose a scene settings JSON file");
        Json json;
        try { json = Json::parse(reinterpret_cast<const char*>(inputs.front().contents.data()),
                                 reinterpret_cast<const char*>(inputs.front().contents.data() + inputs.front().contents.size())); }
        catch (const std::exception& error) { return std::unexpected(error.what()); }
        if (!json.is_object()) return std::unexpected("scene settings JSON must be an object");
        const auto text = json.dump(2);
        const auto bytes = std::as_bytes(std::span(text.data(), text.size()));
        EditorFramework::AssetImportProduct product;
        const auto stem = std::filesystem::path(inputs.front().logicalName).stem().string();
        product.displayPath = stem.empty() ? "Scene Settings" : stem;
        product.entryPoint = "Artifacts/scene-settings.json";
        product.settings = settings.is_object() ? settings : Json::object();
        product.artifacts.push_back({product.entryPoint, {bytes.begin(), bytes.end()}});
        return product;
    };
    auto registered = importers.Register(std::move(importer), runtime);
    if (!registered) return std::unexpected(registered.error());
    return {};
}
} // namespace SampleSceneFeature
