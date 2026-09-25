#pragma once

#include <EditorFramework/Commands.h>
#include <EditorFramework/AssetImport.h>

namespace Aether::EditorFramework
{
using ProjectLifecycleResult = std::expected<void, std::string>;

ProjectLifecycleResult OpenProjectContext(ProjectStateComponent& destination, EditorServicesComponent& services,
    const std::filesystem::path& projectRoot,
    std::shared_ptr<const GameFeatures::RuntimeRegistry> runtime,
    std::shared_ptr<const AssetImporterRegistry> importers, bool writable);

struct ProjectCloseConstraints
{
    bool dirtyDocuments = false;
    bool discardDirtyDocuments = false;
    bool playActive = false;
    bool importTasksActive = false;
};
ProjectLifecycleResult CloseProjectContext(ProjectStateComponent& project, EditorServicesComponent& services,
    const ProjectCloseConstraints& constraints);
}
