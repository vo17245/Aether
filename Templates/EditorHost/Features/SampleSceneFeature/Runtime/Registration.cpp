#include <SampleSceneFeature/Registration.h>
#include <SampleSceneFeature/Codec.h>
#include <SampleSceneFeature/Types.h>

#include <fstream>

namespace
{
Aether::ProjectAssets::Result<Aether::Json> LoadSceneSettingsMetadata(const std::filesystem::path& path)
{
    using namespace Aether;
    try
    {
        std::ifstream stream(path, std::ios::binary);
        if (!stream) return std::unexpected(ProjectAssets::Error{ProjectAssets::ErrorCode::MissingFile, "could not open scene settings", path.string()});
        const auto json = Json::parse(stream);
        if (!json.is_object())
            return std::unexpected(ProjectAssets::Error{ProjectAssets::ErrorCode::CorruptContent, "scene settings artifact must be an object", path.string()});
        return json;
    }
    catch (const std::exception& error)
    {
        return std::unexpected(Aether::ProjectAssets::Error{Aether::ProjectAssets::ErrorCode::CorruptContent, error.what(), path.string()});
    }
}
}

namespace SampleSceneFeature
{
Aether::GameFeatures::CompiledFeature MakeCompiledFeature()
{
    using namespace Aether;
    GameFeatures::CompiledFeature feature;
    feature.descriptor = {"example.sample-scene", 1, 1, {{"example.palette", 1}}};
    feature.registerRuntime = [](GameFeatures::RuntimeFeatureRegistrar& registrar) -> GameFeatures::FeatureResult<void> {
        auto base = GameFeatures::RegisterBaseRuntimeComponents(registrar);
        if (!base) return base;
        auto motion = registrar.RegisterComponent<Motion>();
        if (!motion) return motion;
        auto settings = registrar.RegisterComponent<SceneSettings>();
        if (!settings) return settings;
        auto assetType = registrar.RegisterAssetType({"example.scene-settings", "example.sample-scene", 1,
            LoadSceneSettingsMetadata, [](const std::filesystem::path&) -> ProjectAssets::Result<std::vector<ProjectAssets::ProjectAssetRef>> {
                return std::vector<ProjectAssets::ProjectAssetRef>{};
            }});
        if (!assetType)
            return std::unexpected(GameFeatures::FeatureError{"example.sample-scene", assetType.error().message});
        return {};
    };
    return feature;
}
} // namespace SampleSceneFeature
