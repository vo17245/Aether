#include <EditorFramework/ProjectLifecycle.h>
#include <ProjectAsset/ProjectManifest.h>

#include <fstream>
#include <iterator>
#include <limits>

namespace Aether::EditorFramework
{
namespace
{
bool IsWithin(const std::filesystem::path& root, const std::filesystem::path& candidate)
{
    auto rootIt = root.begin();
    auto candidateIt = candidate.begin();
    for (; rootIt != root.end() && candidateIt != candidate.end() && *rootIt == *candidateIt; ++rootIt, ++candidateIt) {}
    return rootIt == root.end() && candidateIt != candidate.end();
}

std::expected<Json, std::string> ReadJson(const std::filesystem::path& path, std::uint64_t limit)
{
    std::error_code ec;
    if (!std::filesystem::is_regular_file(path, ec) || ec)
        return std::unexpected(std::string("project file is missing or is not a regular file: ") + path.filename().string());
    const auto size = std::filesystem::file_size(path, ec);
    if (ec || size > limit) return std::unexpected(std::string("project file exceeds its size limit: ") + path.filename().string());
    std::ifstream stream(path, std::ios::binary);
    if (!stream) return std::unexpected(std::string("could not open project file: ") + path.filename().string());
    const std::string text{std::istreambuf_iterator<char>(stream), std::istreambuf_iterator<char>()};
    if (text.size() != size) return std::unexpected(std::string("project file changed while being read: ") + path.filename().string());
    try { return Json::parse(text); }
    catch (const std::exception& exception) { return std::unexpected(std::string("invalid project JSON: ") + exception.what()); }
}
}

ProjectLifecycleResult OpenProjectContext(ProjectStateComponent& destination, EditorServicesComponent& services,
    const std::filesystem::path& projectRoot,
    std::shared_ptr<const GameFeatures::RuntimeRegistry> runtime,
    std::shared_ptr<const AssetImporterRegistry> importers, bool writable)
{
    if (!runtime) return std::unexpected(std::string("project open requires a frozen Runtime registry"));
    if (writable && (!importers || !importers->Frozen()))
        return std::unexpected(std::string("writable project open requires a frozen importer registry"));
    if (destination.sessionGeneration == std::numeric_limits<std::uint64_t>::max())
        return std::unexpected(std::string("project session generation is exhausted"));
    std::error_code ec;
    const auto root = std::filesystem::weakly_canonical(projectRoot, ec);
    if (ec || !std::filesystem::is_directory(root, ec) || ec)
        return std::unexpected(std::string("project root is missing or cannot be resolved"));
    const auto manifestPath = std::filesystem::weakly_canonical(root / "project.aether.json", ec);
    if (ec || !IsWithin(root, manifestPath)) return std::unexpected(std::string("project manifest resolves outside project root"));
    const auto catalogPath = std::filesystem::weakly_canonical(root / "Assets" / "catalog.json", ec);
    if (ec || !IsWithin(root, catalogPath)) return std::unexpected(std::string("asset catalog resolves outside project root"));
    auto manifestJson = ReadJson(manifestPath, ProjectAssets::Limits{}.maxManifestBytes);
    if (!manifestJson) return std::unexpected(manifestJson.error());
    auto manifest = ProjectAssets::DecodeProjectManifest(*manifestJson);
    if (!manifest) return std::unexpected(manifest.error().message);
    for (const auto& requirement : manifest->features)
    {
        const auto* feature = runtime->FindFeature(requirement.id);
        if (!feature || feature->version != requirement.version || feature->apiVersion != 1)
            return std::unexpected(std::string("project Runtime Feature is missing or has an incompatible version: ") + requirement.id);
    }
    auto catalogJson = ReadJson(catalogPath, ProjectAssets::Limits{}.maxManifestBytes);
    if (!catalogJson) return std::unexpected(catalogJson.error());
    auto catalogData = ProjectAssets::DecodeCatalog(*catalogJson);
    if (!catalogData) return std::unexpected(catalogData.error().message);
    if (catalogData->projectId != manifest->projectId)
        return std::unexpected(std::string("project manifest and asset catalog have different ProjectIds"));
    auto catalog = ProjectAssets::CatalogSnapshot::Create(std::move(*catalogData), runtime->AssetTypes());
    if (!catalog) return std::unexpected(catalog.error().message);
    std::unique_ptr<ProjectAssets::ProjectWriteLock> lock;
    if (writable)
    {
        auto acquired = ProjectAssets::ProjectWriteLock::Acquire(root);
        if (!acquired) return std::unexpected(acquired.error().message);
        lock = std::move(*acquired);
    }

    // Publish only after manifest, features, catalog and (when requested) the
    // process lock all validate. A failure above preserves the prior context.
    auto publishedCatalog = std::move(*catalog);
    auto destinationRoot = root;
    auto serviceRoot = root;
    destination.open = true;
    destination.writable = writable;
    destination.projectId = manifest->projectId;
    ++destination.sessionGeneration;
    destination.projectRoot.swap(destinationRoot);
    destination.catalog = publishedCatalog;
    destination.importers = std::move(importers);
    destination.writeLock = std::move(lock);
    services.runtime = std::move(runtime);
    services.catalog = std::move(publishedCatalog);
    services.projectRoot.swap(serviceRoot);
    return {};
}

ProjectLifecycleResult CloseProjectContext(ProjectStateComponent& project, EditorServicesComponent& services,
    const ProjectCloseConstraints& constraints)
{
    if (!project.open) return {};
    if (constraints.playActive) return std::unexpected(std::string("stop Play before closing the project"));
    if (constraints.importTasksActive) return std::unexpected(std::string("finish or cancel project imports before closing"));
    if (constraints.dirtyDocuments && !constraints.discardDirtyDocuments)
        return std::unexpected(std::string("dirty documents require Save, Discard, or Cancel before project close"));
    if (project.sessionGeneration == std::numeric_limits<std::uint64_t>::max())
        return std::unexpected(std::string("project session generation is exhausted"));
    project.open = false;
    project.writable = false;
    project.projectId = {};
    project.projectRoot.clear();
    project.catalog.reset();
    project.importers.reset();
    project.writeLock.reset();
    ++project.sessionGeneration;
    services.runtime.reset();
    services.catalog.reset();
    services.projectRoot.clear();
    return {};
}
}
