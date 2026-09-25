#include <PaletteFeature/EditorRegistration.h>

#include <algorithm>
#include <cctype>
#include <span>

namespace PaletteFeature
{
Aether::EditorFramework::EditorFeatureRegistrar::Result RegisterImporter(
    Aether::EditorFramework::AssetImporterRegistry& importers,
    const Aether::GameFeatures::RuntimeRegistry& runtime)
{
    using namespace Aether;
    EditorFramework::AssetImporterDescriptor importer;
    importer.id = "example.palette-importer";
    importer.ownerFeature = "example.palette";
    importer.outputType = "example.palette";
    importer.version = 1;
    importer.extensions = {".json"};
    importer.probe = [](std::span<const std::byte> bytes) {
        return !bytes.empty() && bytes.size() <= ProjectAssets::Limits{}.maxSingleFileBytes;
    };
    importer.import = [](const std::vector<EditorFramework::ImportInputData>& inputs, const Json& settings)
        -> std::expected<EditorFramework::AssetImportProduct, std::string> {
        if (inputs.empty()) return std::unexpected("choose a palette JSON file");
        Json json;
        try { json = Json::parse(reinterpret_cast<const char*>(inputs.front().contents.data()),
                                 reinterpret_cast<const char*>(inputs.front().contents.data() + inputs.front().contents.size())); }
        catch (const std::exception& error) { return std::unexpected(error.what()); }
        if (!json.is_object() || !json.contains("colors") || !json["colors"].is_array())
            return std::unexpected("palette JSON must contain a colors array");
        const auto path = std::filesystem::path(inputs.front().logicalName);
        const auto displayPath = path.stem().empty() ? std::string("Palette") : path.stem().string();
        const auto text = json.dump(2);
        const auto bytes = std::as_bytes(std::span(text.data(), text.size()));
        EditorFramework::AssetImportProduct product;
        product.displayPath = displayPath;
        product.entryPoint = "Artifacts/palette.json";
        product.settings = settings.is_object() ? settings : Json::object();
        product.artifacts.push_back({product.entryPoint, {bytes.begin(), bytes.end()}});
        return product;
    };
    auto registered = importers.Register(std::move(importer), runtime);
    if (!registered) return std::unexpected(registered.error());
    return {};
}
} // namespace PaletteFeature
