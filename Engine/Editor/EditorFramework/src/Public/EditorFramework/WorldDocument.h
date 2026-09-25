#pragma once

#include <EditorFramework/Storage/AtomicReplaceFile.h>
#include <EditorFramework/Commands.h>
#include <GameFeature/ProjectWorld.h>

namespace Aether::EditorFramework
{
using WorldDocumentIoResult = std::expected<void, std::string>;

struct OpenWorldDocumentResult
{
    EntityId entity = entt::null;
    GameFeatures::DocumentId documentId;
    GameFeatures::WorldInstanceId worldInstanceId;
};

WorldDocumentIoResult SaveProjectWorldFile(const World& world, GameFeatures::DocumentId documentId,
    const std::filesystem::path& projectRoot, const GameFeatures::RuntimeRegistry& runtime,
    const ProjectAssets::CatalogSnapshot& catalog, Storage::AtomicFileOperations* fileOperations = nullptr);

Serialization::Result<GameFeatures::DecodedProjectWorld> LoadProjectWorldFile(
    const std::filesystem::path& source, const std::filesystem::path& projectRoot,
    const GameFeatures::RuntimeRegistry& runtime, const ProjectAssets::CatalogSnapshot& catalog,
    std::uint64_t maxFileBytes = 16ull * 1024 * 1024);

std::expected<OpenWorldDocumentResult, std::string> OpenProjectWorldDocument(
    World& editorWorld, const std::filesystem::path& source, const std::filesystem::path& projectRoot,
    std::shared_ptr<const GameFeatures::RuntimeRegistry> runtime,
    std::shared_ptr<const ProjectAssets::CatalogSnapshot> catalog);
}
