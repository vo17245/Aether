#pragma once

#include <PaletteFeature/Types.h>
#include <World/Serialization/ComponentCodecRegistry.h>

namespace Aether::Serialization
{
template<> struct ComponentSerialization<PaletteFeature::PaletteReference>
{
    static constexpr std::string_view Type = "example.palette-reference";
    static constexpr std::uint32_t Version = 1;
    static Result<Json> Serialize(const PaletteFeature::PaletteReference& value, SaveContext&);
    static Result<PaletteFeature::PaletteReference> Deserialize(const Json&, std::uint32_t, LoadContext&);
};
}
