#pragma once

#include <EditorFramework/EditorFeatureRegistrar.h>

namespace PaletteFeature
{
Aether::EditorFramework::EditorFeatureRegistrar::Result RegisterEditor(
    Aether::EditorFramework::EditorFeatureRegistrar& registrar,
    const Aether::GameFeatures::RuntimeRegistry& runtime);
Aether::EditorFramework::EditorFeatureRegistrar::Result RegisterImporter(
    Aether::EditorFramework::AssetImporterRegistry& importers,
    const Aether::GameFeatures::RuntimeRegistry& runtime);
}
