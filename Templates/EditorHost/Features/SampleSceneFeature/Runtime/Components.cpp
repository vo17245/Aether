#include <SampleSceneFeature/Codec.h>

#include <cmath>

namespace Aether::Serialization
{
Result<Json> ComponentSerialization<SampleSceneFeature::Motion>::Serialize(
    const SampleSceneFeature::Motion& value, SaveContext&)
{
    return Json{{"speed", value.speed}};
}

Result<SampleSceneFeature::Motion> ComponentSerialization<SampleSceneFeature::Motion>::Deserialize(
    const Json& json, std::uint32_t version, LoadContext&)
{
    if (version != ComponentSerialization<SampleSceneFeature::Motion>::Version || !json.is_object() ||
        !json.contains("speed") || !json["speed"].is_number())
        return std::unexpected(ArchiveError{ArchiveErrorCode::InvalidFormat, "invalid Motion component"});
    const auto speed = json["speed"].get<float>();
    if (!std::isfinite(speed))
        return std::unexpected(ArchiveError{ArchiveErrorCode::InvalidFormat, "Motion speed must be finite"});
    return SampleSceneFeature::Motion{speed};
}

Result<Json> ComponentSerialization<SampleSceneFeature::SceneSettings>::Serialize(
    const SampleSceneFeature::SceneSettings& value, SaveContext&)
{
    return Json{{"exposure", value.exposure}, {"showGrid", value.showGrid}};
}

Result<SampleSceneFeature::SceneSettings> ComponentSerialization<SampleSceneFeature::SceneSettings>::Deserialize(
    const Json& json, std::uint32_t version, LoadContext&)
{
    if (version != ComponentSerialization<SampleSceneFeature::SceneSettings>::Version || !json.is_object() ||
        !json.contains("exposure") || !json["exposure"].is_number() ||
        !json.contains("showGrid") || !json["showGrid"].is_boolean())
        return std::unexpected(ArchiveError{ArchiveErrorCode::InvalidFormat, "invalid SceneSettings component"});
    const auto exposure = json["exposure"].get<float>();
    if (!std::isfinite(exposure))
        return std::unexpected(ArchiveError{ArchiveErrorCode::InvalidFormat, "scene exposure must be finite"});
    return SampleSceneFeature::SceneSettings{exposure, json["showGrid"].get<bool>()};
}
} // namespace Aether::Serialization
