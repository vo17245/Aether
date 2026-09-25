#include <ProjectAsset/AssetResolver.h>

#include <algorithm>
#include <set>
#include <unordered_set>

namespace Aether::ProjectAssets
{
namespace
{
Error Invalid(ErrorCode code, std::string message, std::string field = {})
{
    return {code, std::move(message), std::move(field)};
}

bool SafeRelativePath(std::string_view value)
{
    if (value.empty() || value.find('\\') != std::string_view::npos || value.find(':') != std::string_view::npos) return false;
    const std::filesystem::path path(value);
    if (path.is_absolute() || path.has_root_name() || path.has_root_directory()) return false;
    for (const auto& part : path)
        if (part == ".." || part == ".") return false;
    return path.generic_string() == value;
}

bool IsNonNegativeInteger(const Json& value)
{
    if (value.is_number_unsigned()) return true;
    return value.is_number_integer() && value.get<std::int64_t>() >= 0;
}

Result<ProjectAssetRef> ReadRef(const Json& json, std::string field)
{
    if (!json.is_object() || !json.contains("projectId") || !json["projectId"].is_string()
        || !json.contains("assetId") || !json["assetId"].is_string())
        return std::unexpected(Invalid(ErrorCode::InvalidFormat, "asset reference requires projectId and assetId", std::move(field)));
    auto project = ProjectId::Parse(json["projectId"].get<std::string>());
    auto asset = AssetId::Parse(json["assetId"].get<std::string>());
    if (!project || !asset)
        return std::unexpected(Invalid(ErrorCode::InvalidFormat, "asset reference contains a non-canonical UUID", std::move(field)));
    return ProjectAssetRef{*project, *asset};
}

Json WriteRef(const ProjectAssetRef& ref)
{
    return {{"projectId", ref.project.ToString()}, {"assetId", ref.asset.ToString()}};
}
}

CatalogSnapshot::CatalogSnapshot(ProjectId project, std::uint64_t generation, std::vector<AssetRecord> records)
    : m_Project(std::move(project)), m_Generation(generation), m_Records(std::move(records))
{
    for (std::size_t i = 0; i < m_Records.size(); ++i)
        m_ById.emplace(m_Records[i].id.ToString(), i);
}

Result<std::shared_ptr<const CatalogSnapshot>> CatalogSnapshot::Create(CatalogData data, const AssetTypeRegistry& types)
{
    if (!data.projectId.IsValid()) return std::unexpected(Invalid(ErrorCode::InvalidArgument, "catalog project ID is invalid"));
    std::unordered_set<std::string> ids;
    for (const auto& record : data.records)
    {
        if (!record.id.IsValid() || !ids.emplace(record.id.ToString()).second)
            return std::unexpected(Invalid(ErrorCode::DuplicateId, "catalog has an invalid or duplicate asset ID"));
        if (!IsStableIdentifier(record.type) || !IsStableIdentifier(record.ownerFeature)
            || record.formatVersion == 0 || record.revision == 0)
            return std::unexpected(Invalid(ErrorCode::InvalidFormat, "asset record has invalid type, owner, format, or revision", record.id.ToString()));
        const auto* type = types.Find(record.type);
        if (!type)
            return std::unexpected(Invalid(ErrorCode::WrongType, "asset record uses an unregistered asset type", record.id.ToString()));
        if (type->ownerFeature != record.ownerFeature || type->formatVersion != record.formatVersion)
            return std::unexpected(Invalid(ErrorCode::WrongType, "asset owner does not match registered type owner", record.id.ToString()));
        if (!SafeRelativePath(record.artifactRoot) || !SafeRelativePath(record.entryPoint))
            return std::unexpected(Invalid(ErrorCode::UnsafePath, "asset paths must be safe relative paths", record.id.ToString()));
        if (record.dependencies.size() > 4096)
            return std::unexpected(Invalid(ErrorCode::LimitExceeded, "asset dependency list exceeds limit", record.id.ToString()));
        if (record.files.size() > 100000)
            return std::unexpected(Invalid(ErrorCode::LimitExceeded, "asset revision file list exceeds limit", record.id.ToString()));
        std::unordered_set<std::string> paths;
        for (const auto& file : record.files)
        {
            if (!SafeRelativePath(file.path) || !paths.emplace(file.path).second)
                return std::unexpected(Invalid(ErrorCode::UnsafePath, "revision file path is unsafe or duplicated", record.id.ToString()));
            if (file.byteSize > 1024ull * 1024 * 1024)
                return std::unexpected(Invalid(ErrorCode::LimitExceeded, "revision file exceeds per-file size limit", record.id.ToString()));
        }
        if (!paths.contains(record.entryPoint))
            return std::unexpected(Invalid(ErrorCode::InvalidFormat, "entry point is not listed in revision files", record.id.ToString()));
        for (const auto& dependency : record.dependencies)
        {
            if (dependency.project != data.projectId)
                return std::unexpected(Invalid(ErrorCode::WrongProject, "asset dependency belongs to another project", record.id.ToString()));
            if (!ids.contains(dependency.asset.ToString()))
            {
                // Later records may satisfy this reference; the complete pass below checks existence.
            }
        }
    }
    for (const auto& record : data.records)
        for (const auto& dependency : record.dependencies)
            if (!ids.contains(dependency.asset.ToString()))
                return std::unexpected(Invalid(ErrorCode::DependencyMissing, "asset dependency is absent from catalog", record.id.ToString()));
    return std::shared_ptr<const CatalogSnapshot>(new CatalogSnapshot(data.projectId, data.generation, std::move(data.records)));
}

const AssetRecord* CatalogSnapshot::Find(AssetId id) const noexcept
{
    const auto it = m_ById.find(id.ToString());
    return it == m_ById.end() ? nullptr : &m_Records[it->second];
}

Result<ResolvedAsset> ResolveAsset(const CatalogSnapshot& catalog, const ProjectAssetRef& reference,
                                   std::string_view expectedType, const std::filesystem::path& projectRoot,
                                   const AssetTypeRegistry& types, std::string_view field)
{
    if (reference.project != catalog.Project())
        return std::unexpected(Invalid(ErrorCode::WrongProject, "asset reference belongs to another project", std::string(field)));
    const auto* record = catalog.Find(reference.asset);
    if (!record) return std::unexpected(Invalid(ErrorCode::MissingAsset, "asset is not in the committed catalog", std::string(field)));
    if (record->type != expectedType)
        return std::unexpected(Invalid(ErrorCode::WrongType, "asset type does not match field requirement", std::string(field)));
    if (!types.Find(record->type))
        return std::unexpected(Invalid(ErrorCode::WrongType, "asset type is not registered", std::string(field)));
    const auto relative = std::filesystem::path(record->artifactRoot) / record->entryPoint;
    if (!SafeRelativePath(relative.generic_string()))
        return std::unexpected(Invalid(ErrorCode::UnsafePath, "resolved asset path escapes its revision root", std::string(field)));
    std::error_code ec;
    const auto root = std::filesystem::weakly_canonical(projectRoot, ec);
    if (ec) return std::unexpected(Invalid(ErrorCode::IoError, "could not resolve project root", std::string(field)));
    const auto candidate = std::filesystem::weakly_canonical(root / relative, ec);
    if (ec) return std::unexpected(Invalid(ErrorCode::IoError, "could not resolve asset entry path", std::string(field)));
    auto rootIt = root.begin(), candidateIt = candidate.begin();
    for (; rootIt != root.end() && candidateIt != candidate.end() && *rootIt == *candidateIt; ++rootIt, ++candidateIt) {}
    if (rootIt != root.end())
        return std::unexpected(Invalid(ErrorCode::UnsafePath, "resolved asset path escapes project root", std::string(field)));
    for (const auto& file : record->files)
    {
        const auto filePath = std::filesystem::weakly_canonical(root / record->artifactRoot / file.path, ec);
        if (ec) return std::unexpected(Invalid(ErrorCode::IoError, "could not resolve revision file", std::string(field)));
        auto revisionRoot = std::filesystem::weakly_canonical(root / record->artifactRoot, ec);
        if (ec) return std::unexpected(Invalid(ErrorCode::IoError, "could not resolve revision root", std::string(field)));
        auto rootPart = revisionRoot.begin(), filePart = filePath.begin();
        for (; rootPart != revisionRoot.end() && filePart != filePath.end() && *rootPart == *filePart; ++rootPart, ++filePart) {}
        if (rootPart != revisionRoot.end())
            return std::unexpected(Invalid(ErrorCode::UnsafePath, "revision file escapes artifact root", std::string(field)));
        if (!std::filesystem::is_regular_file(filePath, ec) || ec)
            return std::unexpected(Invalid(ErrorCode::MissingFile, "revision file is missing", std::string(field)));
        const auto actual = std::filesystem::file_size(filePath, ec);
        if (ec || actual != file.byteSize)
            return std::unexpected(Invalid(ErrorCode::CorruptContent, "revision file size does not match catalog", std::string(field)));
    }
    return ResolvedAsset{reference, record->type, record->revision, candidate};
}

Result<void> ValidateReferenceClosure(const CatalogSnapshot& catalog, const ProjectAssetRef& reference,
                                      const std::filesystem::path& projectRoot,
                                      const AssetTypeRegistry& types, std::string_view field)
{
    std::unordered_set<std::string> visiting, visited;
    auto visit = [&](auto&& self, const AssetRecord& record) -> Result<void> {
        const auto id = record.id.ToString();
        if (visiting.contains(id)) return std::unexpected(Invalid(ErrorCode::DependencyCycle, "asset dependency cycle", std::string(field)));
        if (visited.contains(id)) return {};
        visiting.emplace(id);
        const ProjectAssetRef ref{catalog.Project(), record.id};
        auto resolved = ResolveAsset(catalog, ref, record.type, projectRoot, types, field);
        if (!resolved) return std::unexpected(resolved.error());
        for (const auto& dependency : record.dependencies)
        {
            if (dependency.project != catalog.Project())
                return std::unexpected(Invalid(ErrorCode::WrongProject, "dependency belongs to another project", std::string(field)));
            const auto* target = catalog.Find(dependency.asset);
            if (!target) return std::unexpected(Invalid(ErrorCode::DependencyMissing, "dependency is missing", std::string(field)));
            auto nested = self(self, *target);
            if (!nested) return nested;
        }
        visiting.erase(id);
        visited.emplace(id);
        return {};
    };
    const auto* record = catalog.Find(reference.asset);
    if (reference.project != catalog.Project()) return std::unexpected(Invalid(ErrorCode::WrongProject, "asset reference belongs to another project", std::string(field)));
    if (!record) return std::unexpected(Invalid(ErrorCode::MissingAsset, "asset is not in the committed catalog", std::string(field)));
    return visit(visit, *record);
}

Result<Json> EncodeCatalog(const CatalogSnapshot& catalog, const Limits& limits)
{
    if (catalog.Records().size() > limits.maxRecords)
        return std::unexpected(Invalid(ErrorCode::LimitExceeded, "catalog has too many records"));
    Json json{{"format", "Aether.AssetCatalog"}, {"version", 1}, {"projectId", catalog.Project().ToString()},
              {"generation", catalog.Generation()}, {"assets", Json::array()}};
    for (const auto& record : catalog.Records())
    {
        Json dependencies = Json::array();
        for (const auto& dep : record.dependencies) dependencies.push_back(WriteRef(dep));
        Json sources = Json::array();
        for (const auto& source : record.provenance.sourceFiles)
            sources.push_back({{"logicalName", source.logicalName}, {"originalPath", source.originalPath.generic_string()}});
        Json files = Json::array();
        for (const auto& file : record.files)
            files.push_back({{"path", file.path}, {"byteSize", file.byteSize}});
        json["assets"].push_back({{"id", record.id.ToString()}, {"type", record.type}, {"ownerFeature", record.ownerFeature},
            {"formatVersion", record.formatVersion}, {"revision", record.revision}, {"displayPath", record.displayPath},
            {"artifactRoot", record.artifactRoot}, {"entryPoint", record.entryPoint}, {"dependencies", dependencies}, {"files", files},
            {"provenance", {{"importerId", record.provenance.importerId}, {"importerVersion", record.provenance.importerVersion},
                {"settings", record.provenance.settings}, {"sourceFiles", sources}}}});
    }
    if (json.dump().size() > limits.maxManifestBytes)
        return std::unexpected(Invalid(ErrorCode::LimitExceeded, "catalog exceeds byte limit"));
    return json;
}

Result<CatalogData> DecodeCatalog(const Json& json, const Limits& limits)
{
    if (json.dump().size() > limits.maxManifestBytes)
        return std::unexpected(Invalid(ErrorCode::LimitExceeded, "catalog exceeds byte limit"));
    if (!json.is_object() || json.value("format", std::string{}) != "Aether.AssetCatalog")
        return std::unexpected(Invalid(ErrorCode::InvalidFormat, "expected Aether.AssetCatalog", "/format"));
    if (!json.contains("version") || !(json["version"].is_number_integer() || json["version"].is_number_unsigned()) || json["version"] != 1)
        return std::unexpected(Invalid(ErrorCode::UnsupportedVersion, "unsupported catalog version", "/version"));
    if (!json.contains("projectId") || !json["projectId"].is_string() || !json.contains("generation")
        || !IsNonNegativeInteger(json["generation"])
        || !json.contains("assets") || !json["assets"].is_array())
        return std::unexpected(Invalid(ErrorCode::InvalidFormat, "catalog header or assets array is invalid"));
    auto project = ProjectId::Parse(json["projectId"].get<std::string>());
    if (!project) return std::unexpected(Invalid(ErrorCode::InvalidFormat, "catalog project ID is not canonical", "/projectId"));
    if (json["assets"].size() > limits.maxRecords) return std::unexpected(Invalid(ErrorCode::LimitExceeded, "catalog has too many records"));
    CatalogData result{*project, json["generation"].get<std::uint64_t>(), {}};
    for (std::size_t i = 0; i < json["assets"].size(); ++i)
    {
        const auto& item = json["assets"][i];
        const auto field = "/assets/" + std::to_string(i);
        if (!item.is_object()) return std::unexpected(Invalid(ErrorCode::InvalidFormat, "asset record must be an object", field));
        try
        {
            auto id = item.at("id").get<std::string>();
            auto parsedId = AssetId::Parse(id);
            if (!parsedId) return std::unexpected(Invalid(ErrorCode::InvalidFormat, "asset ID is not canonical", field + "/id"));
            AssetRecord record;
            record.id = *parsedId;
            record.type = item.at("type").get<std::string>();
            record.ownerFeature = item.at("ownerFeature").get<std::string>();
            record.formatVersion = item.at("formatVersion").get<std::uint32_t>();
            record.revision = item.at("revision").get<std::uint64_t>();
            record.displayPath = item.at("displayPath").get<std::string>();
            record.artifactRoot = item.at("artifactRoot").get<std::string>();
            record.entryPoint = item.at("entryPoint").get<std::string>();
            if (!item.at("files").is_array() || item.at("files").size() > limits.maxFilesPerRevision)
                return std::unexpected(Invalid(ErrorCode::LimitExceeded, "revision files exceed limit", field + "/files"));
            for (const auto& file : item.at("files"))
            {
                if (!file.is_object() || !file.contains("path") || !file["path"].is_string()
                    || !file.contains("byteSize") || !IsNonNegativeInteger(file["byteSize"]))
                    return std::unexpected(Invalid(ErrorCode::InvalidFormat, "revision file requires safe path and non-negative byteSize", field + "/files"));
                record.files.push_back({file.at("path").get<std::string>(), file.at("byteSize").get<std::uint64_t>()});
            }
            if (!item.at("dependencies").is_array() || item.at("dependencies").size() > limits.maxDependenciesPerAsset)
                return std::unexpected(Invalid(ErrorCode::LimitExceeded, "asset dependencies exceed limit", field + "/dependencies"));
            for (std::size_t n = 0; n < item.at("dependencies").size(); ++n)
            {
                auto dependency = ReadRef(item.at("dependencies")[n], field + "/dependencies/" + std::to_string(n));
                if (!dependency) return std::unexpected(dependency.error());
                record.dependencies.push_back(std::move(*dependency));
            }
            const auto& provenance = item.at("provenance");
            record.provenance.importerId = provenance.at("importerId").get<std::string>();
            record.provenance.importerVersion = provenance.at("importerVersion").get<std::uint32_t>();
            record.provenance.settings = provenance.at("settings");
            if (!provenance.at("sourceFiles").is_array() || provenance.at("sourceFiles").size() > limits.maxFilesPerRevision)
                return std::unexpected(Invalid(ErrorCode::LimitExceeded, "source file list exceeds limit", field + "/provenance/sourceFiles"));
            for (const auto& source : provenance.at("sourceFiles"))
                record.provenance.sourceFiles.push_back({source.at("logicalName").get<std::string>(), source.at("originalPath").get<std::string>()});
            result.records.push_back(std::move(record));
        }
        catch (const std::exception& exception)
        {
            return std::unexpected(Invalid(ErrorCode::InvalidFormat, exception.what(), field));
        }
    }
    return result;
}
}
