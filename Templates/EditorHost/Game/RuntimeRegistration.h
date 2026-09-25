#pragma once

#include <GameFeature/RuntimeFeatureRegistrar.h>

namespace SampleGame
{
Aether::ProjectAssets::ProjectManifest MakeProjectManifest();
Aether::GameFeatures::FeatureResult<std::shared_ptr<const Aether::GameFeatures::RuntimeRegistry>>
BuildRuntimeRegistry(const Aether::ProjectAssets::ProjectManifest& manifest);
}
