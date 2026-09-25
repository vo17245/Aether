#pragma once

#include <GameFeature/EntityMetadata.h>
#include <GameFeature/RuntimeFeatureRegistrar.h>
#include <ProjectAsset/AssetResolver.h>

namespace Aether::GameFeatures
{
struct DecodedProjectWorld
{
    DocumentId documentId;
    std::unique_ptr<World> world;
};

Serialization::Result<Json> EncodeProjectWorld(const World& world, DocumentId documentId,
    const RuntimeRegistry& runtime, const ProjectAssets::CatalogSnapshot& catalog,
    const std::filesystem::path& projectRoot);
Serialization::Result<DecodedProjectWorld> DecodeProjectWorld(const Json& json,
    const RuntimeRegistry& runtime, const ProjectAssets::CatalogSnapshot& catalog,
    const std::filesystem::path& projectRoot);
Serialization::Result<DecodedProjectWorld> CloneProjectWorldForPlay(const World& authoringWorld,
    DocumentId documentId, const RuntimeRegistry& runtime,
    const ProjectAssets::CatalogSnapshot& catalog, const std::filesystem::path& projectRoot);
}
