#include "RuntimeRegistration.h"

#include <PaletteFeature/Registration.h>
#include <SampleSceneFeature/Registration.h>

#include <stdexcept>

namespace SampleGame
{
Aether::ProjectAssets::ProjectManifest MakeProjectManifest()
{
    const auto projectId = Aether::ProjectAssets::ProjectId::Parse("00112233-4455-6677-8899-aabbccddeeff");
    if (!projectId) throw std::logic_error("template ProjectId literal is invalid");
    return {*projectId,
        {{"example.palette", 1}, {"example.sample-scene", 1}}};
}

Aether::GameFeatures::FeatureResult<std::shared_ptr<const Aether::GameFeatures::RuntimeRegistry>>
BuildRuntimeRegistry(const Aether::ProjectAssets::ProjectManifest& manifest)
{
    return Aether::GameFeatures::RuntimeRegistry::Build(manifest,
        {PaletteFeature::MakeCompiledFeature(), SampleSceneFeature::MakeCompiledFeature()});
}
}
