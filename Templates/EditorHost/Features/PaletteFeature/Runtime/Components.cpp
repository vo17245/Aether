#include <PaletteFeature/Codec.h>

namespace Aether::Serialization
{
Result<Json> ComponentSerialization<PaletteFeature::PaletteReference>::Serialize(
    const PaletteFeature::PaletteReference& value, SaveContext&)
{
    if (!value.value) return Json(nullptr);
    return Json{{"projectId", value.value->project.ToString()}, {"assetId", value.value->asset.ToString()}};
}

Result<PaletteFeature::PaletteReference> ComponentSerialization<PaletteFeature::PaletteReference>::Deserialize(
    const Json& json, std::uint32_t version, LoadContext&)
{
    if (version != ComponentSerialization<PaletteFeature::PaletteReference>::Version)
        return std::unexpected(ArchiveError{ArchiveErrorCode::UnsupportedVersion, "unsupported PaletteReference version"});
    if (json.is_null()) return PaletteFeature::PaletteReference{};
    if (!json.is_object() || !json.contains("projectId") || !json["projectId"].is_string() ||
        !json.contains("assetId") || !json["assetId"].is_string())
        return std::unexpected(ArchiveError{ArchiveErrorCode::InvalidFormat, "PaletteReference requires projectId and assetId"});
    auto project = ProjectAssets::ProjectId::Parse(json["projectId"].get<std::string>());
    auto asset = ProjectAssets::AssetId::Parse(json["assetId"].get<std::string>());
    if (!project || !asset)
        return std::unexpected(ArchiveError{ArchiveErrorCode::InvalidFormat, "PaletteReference contains a non-canonical UUID"});
    return PaletteFeature::PaletteReference{ProjectAssets::ProjectAssetRef{*project, *asset}};
}
} // namespace Aether::Serialization
