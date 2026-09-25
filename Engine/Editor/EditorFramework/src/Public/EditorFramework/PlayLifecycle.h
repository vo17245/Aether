#pragma once

#include <EditorFramework/EditorComponents.h>
#include <GameFeature/ProjectWorld.h>

namespace Aether::EditorFramework
{
struct PlayError { std::string message; };
template<class T> using PlayResult = std::expected<T, PlayError>;

struct PlayStartConstraints
{
    bool dirtyAssetDocuments = false;
    bool importCommitPending = false;
    bool activeEditTransaction = false;
};

PlayResult<void> StartPlay(PlaySessionComponent& session, const World& authoringWorld,
    GameFeatures::DocumentId documentId, const GameFeatures::RuntimeRegistry& runtime,
    std::shared_ptr<const ProjectAssets::CatalogSnapshot> catalog,
    const std::filesystem::path& projectRoot, const PlayStartConstraints& constraints = {});
PlayResult<void> PausePlay(PlaySessionComponent& session);
PlayResult<void> ResumePlay(PlaySessionComponent& session);
PlayResult<void> StepPlay(PlaySessionComponent& session);
PlayResult<std::uint32_t> AdvancePlay(PlaySessionComponent& session, double deltaTimeSeconds);
void StopPlay(PlaySessionComponent& session) noexcept;
}
