#include <EditorFramework/PlayLifecycle.h>

#include <algorithm>
#include <cmath>

namespace Aether::EditorFramework
{
namespace
{
PlayError StateError(std::string message) { return {std::move(message)}; }
}

PlayResult<void> StartPlay(PlaySessionComponent& session, const World& authoringWorld,
    GameFeatures::DocumentId documentId, const GameFeatures::RuntimeRegistry& runtime,
    std::shared_ptr<const ProjectAssets::CatalogSnapshot> catalog,
    const std::filesystem::path& projectRoot, const PlayStartConstraints& constraints)
{
    if (session.state != PlayState::Stopped)
        return std::unexpected(StateError("Play session must be stopped before it can start"));
    if (constraints.dirtyAssetDocuments || constraints.importCommitPending || constraints.activeEditTransaction)
        return std::unexpected(StateError("Play cannot start while asset edits, imports, or an edit transaction are pending"));
    if (!catalog) return std::unexpected(StateError("Play requires a project catalog snapshot"));
    session.state = PlayState::Starting;
    session.error.clear();
    auto clone = GameFeatures::CloneProjectWorldForPlay(authoringWorld, documentId, runtime, *catalog, projectRoot);
    if (!clone)
    {
        session.state = PlayState::Failed;
        session.error = clone.error().message;
        return std::unexpected(StateError(session.error));
    }

    auto mountScope = std::make_unique<GameFeatures::FeatureMountScope>();
    GameFeatures::WorldMountContext context;
    context.role = GameFeatures::WorldRole::Play;
    context.worldInstanceId = GameFeatures::WorldInstanceId::Create();
    context.catalog = catalog;
    auto mounted = mountScope->Mount(*clone->world, context, runtime.Systems());
    if (!mounted)
    {
        session.state = PlayState::Failed;
        session.error = mounted.error().message;
        return std::unexpected(StateError(session.error));
    }

    session.playWorld = std::move(clone->world);
    session.mountScope = std::move(mountScope);
    session.instanceId = context.worldInstanceId;
    session.catalog = std::move(catalog);
    session.clock.accumulatorSeconds = 0.0;
    session.clock.tickIndex = 0;
    session.state = PlayState::Playing;
    return {};
}

PlayResult<void> PausePlay(PlaySessionComponent& session)
{
    if (session.state != PlayState::Playing) return std::unexpected(StateError("only a playing session can be paused"));
    session.state = PlayState::Paused;
    return {};
}

PlayResult<void> ResumePlay(PlaySessionComponent& session)
{
    if (session.state != PlayState::Paused) return std::unexpected(StateError("only a paused session can be resumed"));
    session.state = PlayState::Playing;
    return {};
}

PlayResult<void> StepPlay(PlaySessionComponent& session)
{
    if (session.state != PlayState::Paused || !session.playWorld)
        return std::unexpected(StateError("Step requires a paused Play session"));
    try
    {
        session.playWorld->OnUpdatePhase(SystemUpdatePhase::Simulation, static_cast<float>(session.clock.fixedStepSeconds));
        ++session.clock.tickIndex;
        session.clock.accumulatorSeconds = 0.0;
        return {};
    }
    catch (const std::exception& exception)
    {
        session.state = PlayState::Failed;
        session.error = exception.what();
        return std::unexpected(StateError(session.error));
    }
}

PlayResult<std::uint32_t> AdvancePlay(PlaySessionComponent& session, double deltaTimeSeconds)
{
    if (session.state != PlayState::Playing || !session.playWorld)
        return std::uint32_t{0};
    if (!std::isfinite(deltaTimeSeconds) || deltaTimeSeconds < 0.0)
        return std::unexpected(StateError("Play delta time must be finite and non-negative"));
    if (!std::isfinite(session.clock.fixedStepSeconds) || session.clock.fixedStepSeconds <= 0.0 || session.clock.maxCatchUpSteps == 0)
        return std::unexpected(StateError("simulation clock configuration is invalid"));
    session.clock.accumulatorSeconds += deltaTimeSeconds;
    const auto maxAccumulated = session.clock.fixedStepSeconds * session.clock.maxCatchUpSteps;
    session.clock.accumulatorSeconds = std::min(session.clock.accumulatorSeconds, maxAccumulated);
    std::uint32_t steps = 0;
    try
    {
        while (steps < session.clock.maxCatchUpSteps && session.clock.accumulatorSeconds >= session.clock.fixedStepSeconds)
        {
            session.playWorld->OnUpdatePhase(SystemUpdatePhase::Simulation, static_cast<float>(session.clock.fixedStepSeconds));
            session.clock.accumulatorSeconds -= session.clock.fixedStepSeconds;
            ++session.clock.tickIndex;
            ++steps;
        }
    }
    catch (const std::exception& exception)
    {
        session.state = PlayState::Failed;
        session.error = exception.what();
        return std::unexpected(StateError(session.error));
    }
    return steps;
}

void StopPlay(PlaySessionComponent& session) noexcept
{
    if (session.mountScope)
    {
        session.mountScope->Shutdown();
        session.mountScope.reset();
    }
    session.playWorld.reset();
    session.catalog.reset();
    session.instanceId = {};
    session.clock.accumulatorSeconds = 0.0;
    session.state = PlayState::Stopped;
    session.error.clear();
}
}
