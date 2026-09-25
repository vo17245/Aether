#pragma once

#include <SampleSceneFeature/Types.h>
#include <World/Serialization/ComponentCodecRegistry.h>

namespace Aether::Serialization
{
template<> struct ComponentSerialization<SampleSceneFeature::Motion>
{
    static constexpr std::string_view Type = "example.motion";
    static constexpr std::uint32_t Version = 1;
    static Result<Json> Serialize(const SampleSceneFeature::Motion&, SaveContext&);
    static Result<SampleSceneFeature::Motion> Deserialize(const Json&, std::uint32_t, LoadContext&);
};

template<> struct ComponentSerialization<SampleSceneFeature::SceneSettings>
{
    static constexpr std::string_view Type = "example.scene-settings-component";
    static constexpr std::uint32_t Version = 1;
    static Result<Json> Serialize(const SampleSceneFeature::SceneSettings&, SaveContext&);
    static Result<SampleSceneFeature::SceneSettings> Deserialize(const Json&, std::uint32_t, LoadContext&);
};
}
