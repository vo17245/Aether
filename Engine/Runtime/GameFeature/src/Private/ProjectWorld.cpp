#include <GameFeature/ProjectWorld.h>

#include <unordered_map>
#include <unordered_set>

namespace Aether::GameFeatures
{
namespace
{
Serialization::ArchiveError Error(Serialization::ArchiveErrorCode code, std::string message, std::string pointer = {})
{
    Serialization::ArchiveError error;
    error.code = code;
    error.message = std::move(message);
    error.jsonPointer = std::move(pointer);
    return error;
}

Serialization::Result<void> ValidateProjectWorldImpl(const World& world, const RuntimeRegistry& runtime,
    const ProjectAssets::CatalogSnapshot& catalog, const std::filesystem::path& projectRoot)
{
    std::unordered_map<std::string, EntityId> ids;
    const auto entities = world.Entities();
    for (const auto entity : entities)
    {
        if (world.HasComponent<Serialization::ExcludeFromArchiveComponent>(entity)) continue;
        if (!world.HasComponent<PersistentEntityIdComponent>(entity))
            return std::unexpected(Error(Serialization::ArchiveErrorCode::InvalidFormat,
                "project World entity is missing its persistent ID"));
        const auto& id = world.GetComponent<PersistentEntityIdComponent>(entity).value;
        if (!id.IsValid() || !ids.emplace(id.ToString(), entity).second)
            return std::unexpected(Error(Serialization::ArchiveErrorCode::InvalidFormat,
                "project World has an invalid or duplicate persistent entity ID"));
    }

    for (const auto entity : entities)
    {
        if (world.HasComponent<Serialization::ExcludeFromArchiveComponent>(entity)) continue;
        for (const auto& [type, descriptor] : runtime.Components())
        {
            (void)type;
            if (!descriptor.hasComponent || !descriptor.hasComponent(world, entity)) continue;
            if (descriptor.validate)
            {
                auto valid = descriptor.validate(world, entity);
                if (!valid) return std::unexpected(valid.error());
            }
            if (descriptor.visitEntityReferences)
            {
                bool missing = false;
                descriptor.visitEntityReferences(world, entity, [&](const PersistentEntityId& target) {
                    if (!target.IsValid() || !ids.contains(target.ToString())) missing = true;
                });
                if (missing)
                    return std::unexpected(Error(Serialization::ArchiveErrorCode::InvalidFormat,
                        "component references a missing persistent entity"));
            }
            if (!descriptor.assetFields.empty())
            {
                std::unordered_map<std::string, const AssetFieldContract*> expected;
                for (const auto& field : descriptor.assetFields) expected.emplace(field.fieldId, &field);
                std::unordered_set<std::string> visited;
                descriptor.visitAssetReferences(world, entity, [&](const AssetReferenceValue& value) {
                    const auto found = expected.find(value.fieldId);
                    if (found == expected.end() || found->second->expectedType != value.expectedType || !visited.emplace(value.fieldId).second)
                        throw std::logic_error("asset reference visitor does not match its registered schema");
                    if (!value.value)
                    {
                        if (!found->second->nullable)
                            throw std::logic_error("required asset field is empty: " + value.fieldId);
                        return;
                    }
                    auto resolved = ProjectAssets::ResolveAsset(catalog, *value.value, value.expectedType, projectRoot,
                                                                runtime.AssetTypes(), value.fieldId);
                    if (!resolved) throw std::runtime_error(resolved.error().message + " at " + value.fieldId);
                    auto closure = ProjectAssets::ValidateReferenceClosure(catalog, *value.value, projectRoot,
                                                                           runtime.AssetTypes(), value.fieldId);
                    if (!closure) throw std::runtime_error(closure.error().message + " at " + value.fieldId);
                });
                if (visited.size() != expected.size())
                    return std::unexpected(Error(Serialization::ArchiveErrorCode::InvalidFormat,
                        "asset reference visitor omitted one or more declared fields"));
            }
        }
    }

    std::unordered_map<std::string, std::uint8_t> parentState;
    std::function<bool(const std::string&)> visitParent = [&](const std::string& id) {
        const auto state = parentState[id];
        if (state == 1) return false;
        if (state == 2) return true;
        parentState[id] = 1;
        const auto entity = ids.at(id);
        if (world.HasComponent<ParentComponent>(entity))
        {
            const auto& parent = world.GetComponent<ParentComponent>(entity).parent;
            if (!ids.contains(parent.ToString()) || !visitParent(parent.ToString())) return false;
        }
        parentState[id] = 2;
        return true;
    };
    for (const auto& [id, entity] : ids)
    {
        (void)entity;
        if (!visitParent(id))
            return std::unexpected(Error(Serialization::ArchiveErrorCode::InvalidFormat,
                "Parent references are missing or form a cycle"));
    }
    return {};
}

Serialization::Result<void> ValidateProjectWorld(const World& world, const RuntimeRegistry& runtime,
    const ProjectAssets::CatalogSnapshot& catalog, const std::filesystem::path& projectRoot)
{
    try
    {
        return ValidateProjectWorldImpl(world, runtime, catalog, projectRoot);
    }
    catch (const std::exception& exception)
    {
        return std::unexpected(Error(Serialization::ArchiveErrorCode::CodecFailure, exception.what()));
    }
    catch (...)
    {
        return std::unexpected(Error(Serialization::ArchiveErrorCode::CodecFailure, "unknown failure while validating project World"));
    }
}

Serialization::Result<DocumentId> ReadDocumentId(const Json& json)
{
    if (!json.is_object() || !json.contains("documentId") || !json["documentId"].is_string())
        return std::unexpected(Error(Serialization::ArchiveErrorCode::InvalidFormat, "documentId must be a UUID string", "/documentId"));
    auto id = DocumentId::Parse(json["documentId"].get<std::string>());
    if (!id) return std::unexpected(Error(Serialization::ArchiveErrorCode::InvalidFormat, "documentId is not a canonical UUID", "/documentId"));
    return *id;
}
}

Serialization::Result<Json> EncodeProjectWorld(const World& world, DocumentId documentId,
    const RuntimeRegistry& runtime, const ProjectAssets::CatalogSnapshot& catalog,
    const std::filesystem::path& projectRoot)
{
    if (!documentId.IsValid()) return std::unexpected(Error(Serialization::ArchiveErrorCode::InvalidArgument, "document ID is invalid"));
    auto valid = ValidateProjectWorld(world, runtime, catalog, projectRoot);
    if (!valid) return std::unexpected(valid.error());
    Serialization::SaveContext context;
    auto worldJson = world.Serialize(runtime.Codecs(), context);
    if (!worldJson) return std::unexpected(worldJson.error());
    return Json{{"format", "Aether.ProjectWorld"}, {"version", 1},
                {"projectId", catalog.Project().ToString()}, {"documentId", documentId.ToString()},
                {"world", std::move(*worldJson)}};
}

Serialization::Result<DecodedProjectWorld> DecodeProjectWorld(const Json& json,
    const RuntimeRegistry& runtime, const ProjectAssets::CatalogSnapshot& catalog,
    const std::filesystem::path& projectRoot)
{
    if (!json.is_object() || !json.contains("format") || !json["format"].is_string()
        || json["format"] != "Aether.ProjectWorld")
        return std::unexpected(Error(Serialization::ArchiveErrorCode::InvalidFormat, "expected Aether.ProjectWorld", "/format"));
    if (!json.contains("version") || !(json["version"].is_number_integer() || json["version"].is_number_unsigned()) || json["version"] != 1)
        return std::unexpected(Error(Serialization::ArchiveErrorCode::UnsupportedVersion, "unsupported project World version", "/version"));
    if (!json.contains("projectId") || !json["projectId"].is_string() || json["projectId"] != catalog.Project().ToString())
        return std::unexpected(Error(Serialization::ArchiveErrorCode::InvalidFormat, "project World belongs to another project", "/projectId"));
    auto documentId = ReadDocumentId(json);
    if (!documentId) return std::unexpected(documentId.error());
    if (!json.contains("world")) return std::unexpected(Error(Serialization::ArchiveErrorCode::InvalidFormat, "project World has no World archive", "/world"));
    Serialization::LoadContext context;
    auto world = World::Deserialize(json["world"], runtime.Codecs(), context);
    if (!world) return std::unexpected(world.error());
    auto valid = ValidateProjectWorld(**world, runtime, catalog, projectRoot);
    if (!valid) return std::unexpected(valid.error());
    return DecodedProjectWorld{*documentId, std::move(*world)};
}

Serialization::Result<DecodedProjectWorld> CloneProjectWorldForPlay(const World& authoringWorld,
    DocumentId documentId, const RuntimeRegistry& runtime,
    const ProjectAssets::CatalogSnapshot& catalog, const std::filesystem::path& projectRoot)
{
    auto document = EncodeProjectWorld(authoringWorld, documentId, runtime, catalog, projectRoot);
    if (!document) return std::unexpected(document.error());
    return DecodeProjectWorld(*document, runtime, catalog, projectRoot);
}
}
