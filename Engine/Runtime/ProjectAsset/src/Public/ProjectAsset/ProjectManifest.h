#pragma once

#include <ProjectAsset/Types.h>

namespace Aether::ProjectAssets
{
struct FeatureRequirement
{
    FeatureId id;
    std::uint32_t version = 0;
};

struct ProjectManifest
{
    ProjectId projectId;
    std::vector<FeatureRequirement> features;
};

Result<ProjectManifest> DecodeProjectManifest(const Json& json, const Limits& limits = {});
Result<Json> EncodeProjectManifest(const ProjectManifest& manifest, const Limits& limits = {});
}
