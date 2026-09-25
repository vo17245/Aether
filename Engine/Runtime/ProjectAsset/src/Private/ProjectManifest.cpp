#include <ProjectAsset/ProjectManifest.h>

#include <limits>
#include <unordered_set>

namespace Aether::ProjectAssets
{
namespace
{
Error Invalid(std::string message, std::string field = {})
{
    return {ErrorCode::InvalidFormat, std::move(message), std::move(field)};
}
}

Result<ProjectManifest> DecodeProjectManifest(const Json& json, const Limits& limits)
{
    if (json.dump().size() > limits.maxManifestBytes)
        return std::unexpected(Error{ErrorCode::LimitExceeded, "project manifest exceeds byte limit"});
    if (!json.is_object() || json.value("format", std::string{}) != "Aether.Project")
        return std::unexpected(Invalid("expected Aether.Project manifest", "/format"));
    if (!json.contains("version") || !(json["version"].is_number_integer() || json["version"].is_number_unsigned()) || json["version"] != 1)
        return std::unexpected(Error{ErrorCode::UnsupportedVersion, "unsupported project manifest version", "/version"});
    if (!json.contains("projectId") || !json["projectId"].is_string())
        return std::unexpected(Invalid("projectId must be a UUID string", "/projectId"));
    auto projectId = ProjectId::Parse(json["projectId"].get<std::string>());
    if (!projectId) return std::unexpected(Invalid("projectId is not a canonical UUID", "/projectId"));
    if (!json.contains("features") || !json["features"].is_array() || json["features"].size() > limits.maxRecords)
        return std::unexpected(Invalid("features must be a bounded array", "/features"));
    ProjectManifest result{*projectId, {}};
    std::unordered_set<std::string> seen;
    for (std::size_t i = 0; i < json["features"].size(); ++i)
    {
        const auto& item = json["features"][i];
        const auto field = "/features/" + std::to_string(i);
        if (!item.is_object() || !item.contains("id") || !item["id"].is_string()
            || !item.contains("version") || !(item["version"].is_number_integer() || item["version"].is_number_unsigned()))
            return std::unexpected(Invalid("feature requires id and version", field));
        auto id = item["id"].get<std::string>();
        std::uint64_t rawVersion = 0;
        try { rawVersion = item["version"].get<std::uint64_t>(); }
        catch (...) { return std::unexpected(Invalid("feature version is out of range", field + "/version")); }
        if (rawVersion > std::numeric_limits<std::uint32_t>::max())
            return std::unexpected(Invalid("feature version is out of range", field + "/version"));
        const auto version = static_cast<std::uint32_t>(rawVersion);
        if (!IsStableIdentifier(id) || version == 0 || !seen.emplace(id).second)
            return std::unexpected(Invalid("feature id/version is invalid or duplicated", field));
        result.features.push_back({std::move(id), version});
    }
    return result;
}

Result<Json> EncodeProjectManifest(const ProjectManifest& manifest, const Limits& limits)
{
    if (!manifest.projectId.IsValid() || manifest.features.size() > limits.maxRecords)
        return std::unexpected(Error{ErrorCode::InvalidArgument, "invalid project manifest"});
    Json json{{"format", "Aether.Project"}, {"version", 1}, {"projectId", manifest.projectId.ToString()}, {"features", Json::array()}};
    std::unordered_set<std::string> seen;
    for (const auto& feature : manifest.features)
    {
        if (!IsStableIdentifier(feature.id) || feature.version == 0 || !seen.emplace(feature.id).second)
            return std::unexpected(Error{ErrorCode::InvalidArgument, "feature id/version is invalid or duplicated", feature.id});
        json["features"].push_back({{"id", feature.id}, {"version", feature.version}});
    }
    if (json.dump().size() > limits.maxManifestBytes)
        return std::unexpected(Error{ErrorCode::LimitExceeded, "project manifest exceeds byte limit"});
    return json;
}
}
