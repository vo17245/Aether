#pragma once

#include <GameFeature/Types.h>

#include <cstdint>
#include <vector>

namespace Aether::GameFeatures
{
struct FeatureRequirement
{
    FeatureId id;
    std::uint32_t version = 0;
};

struct FeatureDescriptor
{
    FeatureId id;
    std::uint32_t version = 1;
    std::uint32_t apiVersion = 1;
    std::vector<FeatureRequirement> dependencies;
};
}
