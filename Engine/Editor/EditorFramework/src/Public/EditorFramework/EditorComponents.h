#pragma once

#include <GameFeature/Types.h>
#include <GameFeature/WorldMount.h>
#include <ProjectAsset/AssetResolver.h>
#include <ProjectAsset/ProjectLock.h>
#include <EditorFramework/Viewport.h>
#include <World/World.h>

#include <memory>
#include <optional>
#include <string>

namespace Aether::EditorFramework
{
class AssetImporterRegistry;
struct EditorSessionComponent
{
    std::uint64_t generation = 1;
    bool closing = false;
};

struct ProjectStateComponent
{
    bool open = false;
    bool writable = false;
    ProjectAssets::ProjectId projectId;
    std::uint64_t sessionGeneration = 1;
    std::filesystem::path projectRoot;
    std::shared_ptr<const ProjectAssets::CatalogSnapshot> catalog;
    std::shared_ptr<const AssetImporterRegistry> importers;
    std::unique_ptr<ProjectAssets::ProjectWriteLock> writeLock;
};

enum class DocumentKind { World, Asset };
struct DocumentComponent
{
    GameFeatures::DocumentId id;
    DocumentKind kind = DocumentKind::World;
    bool dirty = false;
    std::string error;
};

struct AssetDocumentComponent
{
    ProjectAssets::ProjectId projectId;
    ProjectAssets::AssetId assetId;
    ProjectAssets::AssetTypeId assetType;
    std::uint64_t baseRevision = 0;
    bool readOnly = true;
};

struct WorldDocumentComponent
{
    std::unique_ptr<World> authoringWorld;
    std::unique_ptr<GameFeatures::FeatureMountScope> mountScope;
    GameFeatures::WorldInstanceId instanceId;
    std::uint64_t revision = 0;
};

enum class PlayState { Stopped, Starting, Playing, Paused, Failed };
struct SimulationClockComponent
{
    double fixedStepSeconds = 1.0 / 60.0;
    double accumulatorSeconds = 0.0;
    std::uint64_t tickIndex = 0;
    std::uint32_t maxCatchUpSteps = 4;
};

struct PlaySessionComponent
{
    PlayState state = PlayState::Stopped;
    std::unique_ptr<World> playWorld;
    std::unique_ptr<GameFeatures::FeatureMountScope> mountScope;
    GameFeatures::WorldInstanceId instanceId;
    std::shared_ptr<const ProjectAssets::CatalogSnapshot> catalog;
    SimulationClockComponent clock;
    std::string error;
};

struct SelectionComponent
{
    GameFeatures::WorldInstanceId world;
    std::optional<GameFeatures::PersistentEntityId> entity;
};

struct ViewportPanelComponent
{
    ViewportModel model;
    bool focused = false;
};

struct RequestResultComponent
{
    std::uint64_t requestId = 0;
    bool complete = false;
    std::string error;
};

struct EditorStatusComponent
{
    std::string message;
    bool isError = false;
};
}
