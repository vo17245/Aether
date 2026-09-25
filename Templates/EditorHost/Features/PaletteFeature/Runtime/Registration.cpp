#include <PaletteFeature/Registration.h>
#include <PaletteFeature/Codec.h>
#include <PaletteFeature/Types.h>

#include <fstream>

namespace
{
Aether::ProjectAssets::Result<Aether::Json> LoadPaletteMetadata(const std::filesystem::path& path)
{
    using namespace Aether;
    try
    {
        std::ifstream stream(path, std::ios::binary);
        if (!stream) return std::unexpected(ProjectAssets::Error{ProjectAssets::ErrorCode::MissingFile, "could not open palette artifact", path.string()});
        const auto json = Json::parse(stream);
        if (!json.is_object() || !json.contains("colors") || !json["colors"].is_array())
            return std::unexpected(ProjectAssets::Error{ProjectAssets::ErrorCode::CorruptContent, "palette artifact requires a colors array", path.string()});
        return json;
    }
    catch (const std::exception& error)
    {
        return std::unexpected(Aether::ProjectAssets::Error{Aether::ProjectAssets::ErrorCode::CorruptContent, error.what(), path.string()});
    }
}
}

namespace PaletteFeature
{
Aether::GameFeatures::CompiledFeature MakeCompiledFeature()
{
    using namespace Aether;
    GameFeatures::CompiledFeature feature;
    feature.descriptor = {"example.palette", 1, 1, {}};
    feature.registerRuntime = [](GameFeatures::RuntimeFeatureRegistrar& registrar) -> GameFeatures::FeatureResult<void> {
        auto component = registrar.RegisterComponent<PaletteReference>(
            {{"palette", "example.palette", true}}, {},
            [](const PaletteReference& value, const auto& visit) {
                visit({"palette", "example.palette", value.value});
            });
        if (!component) return component;
        auto assetType = registrar.RegisterAssetType({"example.palette", "example.palette", 1,
            LoadPaletteMetadata, [](const std::filesystem::path&) -> ProjectAssets::Result<std::vector<ProjectAssets::ProjectAssetRef>> {
                return std::vector<ProjectAssets::ProjectAssetRef>{};
            }});
        if (!assetType)
            return std::unexpected(GameFeatures::FeatureError{"example.palette", assetType.error().message});
        return {};
    };
    return feature;
}
} // namespace PaletteFeature
