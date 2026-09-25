#include <EditorFramework/WorldDocument.h>

#include <fstream>
#include <iterator>

namespace Aether::EditorFramework
{
namespace
{
Serialization::ArchiveError ArchiveError(Serialization::ArchiveErrorCode code, std::string message)
{
    return {code, std::move(message)};
}

bool IsWithin(const std::filesystem::path& root, const std::filesystem::path& candidate)
{
    auto rootIt = root.begin();
    auto candidateIt = candidate.begin();
    for (; rootIt != root.end() && candidateIt != candidate.end() && *rootIt == *candidateIt; ++rootIt, ++candidateIt) {}
    return rootIt == root.end() && candidateIt != candidate.end();
}

std::expected<std::filesystem::path, std::string> WorldsDirectory(const std::filesystem::path& projectRoot)
{
    std::error_code ec;
    std::filesystem::create_directories(projectRoot / "Worlds", ec);
    if (ec) return std::unexpected(std::string("could not create Worlds directory: ") + ec.message());
    const auto root = std::filesystem::weakly_canonical(projectRoot, ec);
    if (ec) return std::unexpected(std::string("could not resolve project root: ") + ec.message());
    const auto worlds = std::filesystem::weakly_canonical(root / "Worlds", ec);
    if (ec || !IsWithin(root, worlds)) return std::unexpected(std::string("Worlds directory resolves outside the project root"));
    return worlds;
}
}

WorldDocumentIoResult SaveProjectWorldFile(const World& world, GameFeatures::DocumentId documentId,
    const std::filesystem::path& projectRoot, const GameFeatures::RuntimeRegistry& runtime,
    const ProjectAssets::CatalogSnapshot& catalog, Storage::AtomicFileOperations* fileOperations)
{
    if (world.IsDispatching()) return std::unexpected(std::string("World documents can only be saved at a safe point"));
    if (!documentId.IsValid()) return std::unexpected(std::string("document ID is invalid"));
    auto worlds = WorldsDirectory(projectRoot);
    if (!worlds) return std::unexpected(worlds.error());
    auto document = GameFeatures::EncodeProjectWorld(world, documentId, runtime, catalog, projectRoot);
    if (!document) return std::unexpected(std::string(document.error().message));
    const auto contents = document->dump(2);
    const auto bytes = std::as_bytes(std::span(contents.data(), contents.size()));
    const auto destination = *worlds / (documentId.ToString() + ".world.json");
    auto written = Storage::AtomicReplaceFile(destination, bytes, fileOperations);
    if (!written) return std::unexpected(std::string(written.error()));
    return {};
}

Serialization::Result<GameFeatures::DecodedProjectWorld> LoadProjectWorldFile(
    const std::filesystem::path& source, const std::filesystem::path& projectRoot,
    const GameFeatures::RuntimeRegistry& runtime, const ProjectAssets::CatalogSnapshot& catalog,
    std::uint64_t maxFileBytes)
{
    using namespace GameFeatures;
    if (maxFileBytes == 0) return std::unexpected(ArchiveError(Serialization::ArchiveErrorCode::InvalidArgument, "world file limit must be non-zero"));
    std::error_code ec;
    const auto project = std::filesystem::weakly_canonical(projectRoot, ec);
    if (ec) return std::unexpected(ArchiveError(Serialization::ArchiveErrorCode::InvalidArgument, "could not resolve project root"));
    const auto worlds = std::filesystem::weakly_canonical(project / "Worlds", ec);
    if (ec || !IsWithin(project, worlds))
        return std::unexpected(ArchiveError(Serialization::ArchiveErrorCode::InvalidArgument, "Worlds directory resolves outside the project root"));
    const auto path = std::filesystem::weakly_canonical(source, ec);
    if (ec || !IsWithin(worlds, path))
        return std::unexpected(ArchiveError(Serialization::ArchiveErrorCode::InvalidArgument, "World document path is outside the project Worlds directory"));
    if (!std::filesystem::is_regular_file(path, ec) || ec)
        return std::unexpected(ArchiveError(Serialization::ArchiveErrorCode::InvalidFormat, "World document is not a regular file"));
    const auto size = std::filesystem::file_size(path, ec);
    if (ec || size > maxFileBytes)
        return std::unexpected(ArchiveError(Serialization::ArchiveErrorCode::ResourceLimitExceeded, "World document exceeds its byte limit"));
    std::ifstream stream(path, std::ios::binary);
    if (!stream) return std::unexpected(ArchiveError(Serialization::ArchiveErrorCode::InvalidFormat, "could not open World document"));
    const std::string contents{std::istreambuf_iterator<char>(stream), std::istreambuf_iterator<char>()};
    if (contents.size() != size) return std::unexpected(ArchiveError(Serialization::ArchiveErrorCode::InvalidFormat, "World document changed while it was being read"));
    Json json;
    try { json = Json::parse(contents); }
    catch (const std::exception& exception)
    {
        return std::unexpected(ArchiveError(Serialization::ArchiveErrorCode::InvalidFormat, std::string("invalid World JSON: ") + exception.what()));
    }
    auto decoded = DecodeProjectWorld(json, runtime, catalog, projectRoot);
    if (!decoded) return decoded;
    if (path.filename() != decoded->documentId.ToString() + ".world.json")
        return std::unexpected(ArchiveError(Serialization::ArchiveErrorCode::InvalidFormat, "World file name does not match its document ID"));
    return decoded;
}

std::expected<OpenWorldDocumentResult, std::string> OpenProjectWorldDocument(
    World& editorWorld, const std::filesystem::path& source, const std::filesystem::path& projectRoot,
    std::shared_ptr<const GameFeatures::RuntimeRegistry> runtime,
    std::shared_ptr<const ProjectAssets::CatalogSnapshot> catalog)
{
    if (!runtime || !catalog)
        return std::unexpected(std::string("opening a World requires Runtime and catalog snapshots"));
    auto decoded = LoadProjectWorldFile(source, projectRoot, *runtime, *catalog);
    if (!decoded) return std::unexpected(decoded.error().message);
    for (const auto entity : editorWorld.Select<DocumentComponent>())
        if (editorWorld.GetComponent<DocumentComponent>(entity).id == decoded->documentId)
            return std::unexpected(std::string("this World document is already open"));

    const auto instanceId = GameFeatures::WorldInstanceId::Create();
    auto mountScope = std::make_unique<GameFeatures::FeatureMountScope>();
    GameFeatures::WorldMountContext mountContext{GameFeatures::WorldRole::Authoring, instanceId, catalog};
    auto mounted = mountScope->Mount(*decoded->world, mountContext, runtime->Systems());
    if (!mounted) return std::unexpected(mounted.error().message);

    const auto entity = editorWorld.CreateEntity();
    editorWorld.AddComponent<DocumentComponent>(entity,
        DocumentComponent{decoded->documentId, DocumentKind::World, false, {}});
    editorWorld.AddComponent<HistoryComponent>(entity);
    editorWorld.AddComponent<WorldDocumentComponent>(entity,
        WorldDocumentComponent{std::move(decoded->world), std::move(mountScope), instanceId, 0});
    PlaySessionComponent play;
    play.catalog = std::move(catalog);
    editorWorld.AddComponent<PlaySessionComponent>(entity, std::move(play));

    for (const auto session : editorWorld.Select<EditorSessionComponent>())
    {
        auto& selection = editorWorld.HasComponent<SelectionComponent>(session)
            ? editorWorld.GetComponent<SelectionComponent>(session)
            : editorWorld.AddComponent<SelectionComponent>(session);
        selection.world = instanceId;
        selection.entity.reset();
        const auto& opened = *editorWorld.GetComponent<WorldDocumentComponent>(entity).authoringWorld;
        const auto entities = opened.Select<GameFeatures::PersistentEntityIdComponent>();
        if (!entities.empty())
            selection.entity = opened.GetComponent<GameFeatures::PersistentEntityIdComponent>(entities.front()).value;
        break;
    }
    return OpenWorldDocumentResult{entity, decoded->documentId, instanceId};
}
}
