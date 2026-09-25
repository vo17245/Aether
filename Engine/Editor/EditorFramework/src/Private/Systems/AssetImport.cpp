#include <EditorFramework/AssetImport.h>
#include <EditorFramework/Commands.h>

#include <algorithm>
#include <cctype>
#include <chrono>
#include <fstream>
#include <limits>
#include <set>

namespace Aether::EditorFramework
{
namespace
{
ProjectAssets::Error ImportError(ProjectAssets::ErrorCode code, std::string message, std::string field = {})
{
    return {code, std::move(message), std::move(field)};
}

bool SafeRelative(std::string_view value)
{
    if (value.empty()) return false;
    const std::filesystem::path path(value);
    if (path.is_absolute() || path.has_root_name() || path.has_root_directory()) return false;
    for (const auto& part : path)
        if (part.empty() || part == "." || part == "..") return false;
    return path.lexically_normal().generic_string() == value;
}

bool SafeDisplayPath(std::string_view value)
{
    if (value.empty() || value.size() > 1024 || value.find('\\') != std::string_view::npos
        || value.find(':') != std::string_view::npos)
        return false;
    const std::filesystem::path path(value);
    if (path.is_absolute() || path.has_root_name() || path.has_root_directory()) return false;
    for (const auto& part : path)
        if (part.empty() || part == "." || part == "..") return false;
    return path.lexically_normal().generic_string() == value;
}

bool IsWithin(const std::filesystem::path& root, const std::filesystem::path& candidate)
{
    auto rootIt = root.begin();
    auto candidateIt = candidate.begin();
    for (; rootIt != root.end() && candidateIt != candidate.end() && *rootIt == *candidateIt; ++rootIt, ++candidateIt) {}
    return rootIt == root.end() && candidateIt != candidate.end();
}

std::string LowerExtension(const std::filesystem::path& path)
{
    auto extension = path.extension().string();
    std::transform(extension.begin(), extension.end(), extension.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return extension;
}

bool IsCancelled(const std::shared_ptr<std::atomic_bool>& cancelled)
{
    return cancelled->load(std::memory_order_relaxed);
}

std::expected<void, std::string> WriteStagedFile(const std::filesystem::path& root, std::string_view relative,
    std::span<const std::byte> contents)
{
    if (!SafeRelative(relative)) return std::unexpected(std::string("revision contains an unsafe relative path: ") + std::string(relative));
    const auto path = root / std::filesystem::path(relative);
    std::error_code ec;
    std::filesystem::create_directories(path.parent_path(), ec);
    if (ec) return std::unexpected(std::string("could not create staging directory: ") + ec.message());
    std::ofstream stream(path, std::ios::binary | std::ios::trunc);
    if (!stream) return std::unexpected(std::string("could not create staged file: ") + path.string());
    if (!contents.empty()) stream.write(reinterpret_cast<const char*>(contents.data()), static_cast<std::streamsize>(contents.size()));
    stream.flush();
    if (!stream) return std::unexpected(std::string("could not write staged file: ") + path.string());
    stream.close();
    if (!stream) return std::unexpected(std::string("could not close staged file: ") + path.string());
    return {};
}

std::expected<PreparedAssetImport, std::string> PrepareImport(AssetImportRequest request,
    AssetImporterDescriptor importer, const std::filesystem::path& projectRoot,
    const ProjectAssets::Limits& limits, const std::shared_ptr<std::atomic_bool>& cancelled)
{
    if (!request.projectId.IsValid() || request.sessionGeneration == 0 || request.importerId != importer.id
        || (request.inputs.empty() && !request.allowNoInputs) || request.inputs.size() > 128 || !request.settings.is_object())
        return std::unexpected("import request header, inputs, or settings are invalid");
    if (IsCancelled(cancelled)) return std::unexpected("asset import was cancelled");
    std::error_code ec;
    const auto root = std::filesystem::weakly_canonical(projectRoot, ec);
    if (ec) return std::unexpected("could not resolve project root: " + ec.message());
    const auto stagingRoot = root / ".aether" / "staging";
    std::filesystem::create_directories(stagingRoot, ec);
    if (ec) return std::unexpected("could not create import staging root: " + ec.message());
    const auto canonicalStaging = std::filesystem::weakly_canonical(stagingRoot, ec);
    if (ec || !IsWithin(root, canonicalStaging)) return std::unexpected("import staging root resolves outside project root");
    const auto transactionId = ProjectAssets::AssetId::Create().ToString();
    const auto stage = canonicalStaging / transactionId;
    if (!std::filesystem::create_directory(stage, ec) || ec)
        return std::unexpected("could not create unique import staging directory: " + ec.message());
    bool keepStage = false;
    struct Cleanup
    {
        std::filesystem::path path;
        bool& keep;
        ~Cleanup() { if (!keep) { std::error_code ignored; std::filesystem::remove_all(path, ignored); } }
    } cleanup{stage, keepStage};

    std::vector<ImportInputData> inputs;
    std::vector<ProjectAssets::ImportSourceFile> sourceRecords;
    std::set<std::string> logicalNames;
    std::uint64_t totalBytes = 0;
    inputs.reserve(request.inputs.size());
    for (const auto& input : request.inputs)
    {
        if (IsCancelled(cancelled)) return std::unexpected("asset import was cancelled");
        std::string logicalName = input.logicalName.empty() ? input.sourcePath.filename().generic_string() : input.logicalName;
        if (!SafeRelative(logicalName) || !logicalNames.emplace(logicalName).second)
            return std::unexpected("import input has an unsafe or duplicate logical name: " + logicalName);
        if (!std::filesystem::is_regular_file(input.sourcePath, ec) || ec)
            return std::unexpected("import input is not a regular file: " + input.sourcePath.string());
        const auto size = std::filesystem::file_size(input.sourcePath, ec);
        if (ec || size > limits.maxSingleFileBytes || size > limits.maxTotalFileBytes - std::min(totalBytes, limits.maxTotalFileBytes))
            return std::unexpected("import input exceeds configured size limits: " + input.sourcePath.string());
        std::ifstream stream(input.sourcePath, std::ios::binary);
        if (!stream) return std::unexpected("could not read import input: " + input.sourcePath.string());
        ImportInputData data;
        data.logicalName = logicalName;
        data.sourcePath = input.sourcePath;
        data.contents.resize(static_cast<std::size_t>(size));
        if (size != 0) stream.read(reinterpret_cast<char*>(data.contents.data()), static_cast<std::streamsize>(size));
        if (!stream || stream.gcount() != static_cast<std::streamsize>(size))
            return std::unexpected("import input changed while it was being read: " + input.sourcePath.string());
        totalBytes += size;
        const std::string sourcePath = "Source/" + logicalName;
        auto staged = WriteStagedFile(stage, sourcePath, data.contents);
        if (!staged) return std::unexpected(staged.error());
        sourceRecords.push_back({logicalName, input.sourcePath});
        inputs.push_back(std::move(data));
    }
    if (IsCancelled(cancelled)) return std::unexpected("asset import was cancelled");
    if (!importer.extensions.empty())
    {
        const auto extension = LowerExtension(request.inputs.front().sourcePath);
        if (std::find(importer.extensions.begin(), importer.extensions.end(), extension) == importer.extensions.end())
            return std::unexpected("selected importer does not accept the first input extension");
    }
    if (importer.probe && !inputs.empty() && !importer.probe(inputs.front().contents))
        return std::unexpected("selected importer rejected the input contents");
    auto product = importer.import(inputs, request.settings);
    if (!product) return std::unexpected(product.error());
    if (IsCancelled(cancelled)) return std::unexpected("asset import was cancelled");
    if (product->artifacts.empty() || (request.inputs.empty() && product->sources.empty()) || !SafeRelative(product->entryPoint)
        || !product->entryPoint.starts_with("Artifacts/") || product->displayPath.empty())
        return std::unexpected("importer output requires a display path, artifact entry point, and artifact files");

    const auto settingsText = request.settings.dump();
    if (settingsText.size() > limits.maxSingleFileBytes
        || settingsText.size() > limits.maxTotalFileBytes - std::min(totalBytes, limits.maxTotalFileBytes))
        return std::unexpected("import settings exceed configured size limits");
    totalBytes += settingsText.size();
    const auto settingsBytes = std::as_bytes(std::span(settingsText.data(), settingsText.size()));
    auto settingsWrite = WriteStagedFile(stage, "Source/import-settings.json", settingsBytes);
    if (!settingsWrite) return std::unexpected(settingsWrite.error());
    std::vector<ProjectAssets::AssetFileRecord> fileRecords;
    fileRecords.reserve(sourceRecords.size() + product->sources.size() + product->artifacts.size() + 1);
    for (const auto& source : sourceRecords)
    {
        const auto relative = "Source/" + source.logicalName;
        fileRecords.push_back({relative, std::filesystem::file_size(stage / relative, ec)});
        if (ec) return std::unexpected("could not inspect staged source file: " + ec.message());
    }
    fileRecords.push_back({"Source/import-settings.json", settingsText.size()});
    std::set<std::string> sourcePaths;
    for (const auto& source : request.inputs) sourcePaths.emplace("Source/" + source.logicalName);
    sourcePaths.emplace("Source/import-settings.json");
    for (const auto& source : product->sources)
    {
        if (!SafeRelative(source.path) || !source.path.starts_with("Source/") || !sourcePaths.emplace(source.path).second)
            return std::unexpected("importer produced an unsafe or duplicate source path: " + source.path);
        if (source.contents.size() > limits.maxSingleFileBytes
            || source.contents.size() > limits.maxTotalFileBytes - std::min(totalBytes, limits.maxTotalFileBytes))
            return std::unexpected("importer source exceeds configured size limits: " + source.path);
        totalBytes += source.contents.size();
        auto staged = WriteStagedFile(stage, source.path, source.contents);
        if (!staged) return std::unexpected(staged.error());
        fileRecords.push_back({source.path, source.contents.size()});
        sourceRecords.push_back({source.path.substr(std::string("Source/").size()), {}});
    }
    std::set<std::string> artifactPaths;
    bool entryFound = false;
    for (const auto& artifact : product->artifacts)
    {
        if (!SafeRelative(artifact.path) || !artifact.path.starts_with("Artifacts/") || !artifactPaths.emplace(artifact.path).second)
            return std::unexpected("importer produced an unsafe or duplicate artifact path: " + artifact.path);
        if (artifact.contents.size() > limits.maxSingleFileBytes
            || artifact.contents.size() > limits.maxTotalFileBytes - std::min(totalBytes, limits.maxTotalFileBytes))
            return std::unexpected("importer artifact exceeds configured size limits: " + artifact.path);
        totalBytes += artifact.contents.size();
        auto staged = WriteStagedFile(stage, artifact.path, artifact.contents);
        if (!staged) return std::unexpected(staged.error());
        fileRecords.push_back({artifact.path, artifact.contents.size()});
        if (artifact.path == product->entryPoint) entryFound = true;
    }
    if (!entryFound) return std::unexpected("importer entry point is absent from its artifact list");
    auto assetId = request.existingAsset.value_or(ProjectAssets::AssetId::Create());
    if (!assetId.IsValid()) return std::unexpected("import target AssetId is invalid");
    ProjectAssets::AssetRecord record;
    record.id = assetId;
    record.type = importer.outputType;
    record.ownerFeature = importer.ownerFeature;
    record.formatVersion = importer.outputFormatVersion;
    record.revision = request.existingAsset ? request.baseRevision + 1 : 1;
    record.displayPath = product->displayPath;
    record.artifactRoot = "Assets/Data/" + assetId.ToString() + "/" + std::to_string(record.revision);
    record.entryPoint = product->entryPoint;
    record.dependencies = std::move(product->dependencies);
    record.files = std::move(fileRecords);
    record.provenance = {importer.id, importer.version, request.settings, std::move(sourceRecords)};
    keepStage = true;
    return PreparedAssetImport{std::move(request), std::move(record), stage, cancelled};
}

void SortReferences(std::vector<ProjectAssets::ProjectAssetRef>& references)
{
    std::sort(references.begin(), references.end(), [](const auto& left, const auto& right) {
        if (left.project != right.project) return left.project.ToString() < right.project.ToString();
        return left.asset.ToString() < right.asset.ToString();
    });
}
}

std::expected<void, std::string> AssetImporterRegistry::Register(AssetImporterDescriptor descriptor,
    const GameFeatures::RuntimeRegistry& runtime)
{
    if (m_Frozen) return std::unexpected(std::string("asset importer registry is frozen"));
    if (!ProjectAssets::IsStableIdentifier(descriptor.id) || !ProjectAssets::IsStableIdentifier(descriptor.ownerFeature)
        || !ProjectAssets::IsStableIdentifier(descriptor.outputType) || descriptor.version == 0 || !descriptor.import)
        return std::unexpected(std::string("asset importer requires stable IDs, a non-zero version, and an import callback"));
    const auto* feature = runtime.FindFeature(descriptor.ownerFeature);
    const auto* type = runtime.AssetTypes().Find(descriptor.outputType);
    if (!feature || !type || type->ownerFeature != descriptor.ownerFeature)
        return std::unexpected(std::string("asset importer output type is not owned by its Runtime Feature"));
    if (descriptor.outputFormatVersion != 0 && descriptor.outputFormatVersion != type->formatVersion)
        return std::unexpected(std::string("asset importer output format differs from Runtime asset type"));
    descriptor.outputFormatVersion = type->formatVersion;
    if (m_Entries.contains(descriptor.id)) return std::unexpected(std::string("asset importer ID is already registered"));
    for (auto& extension : descriptor.extensions)
    {
        if (extension.empty() || extension.front() != '.') return std::unexpected(std::string("importer extension must start with a dot"));
        std::transform(extension.begin(), extension.end(), extension.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    }
    std::sort(descriptor.extensions.begin(), descriptor.extensions.end());
    if (std::adjacent_find(descriptor.extensions.begin(), descriptor.extensions.end()) != descriptor.extensions.end())
        return std::unexpected(std::string("asset importer contains duplicate extensions"));
    m_Entries.emplace(descriptor.id, std::move(descriptor));
    return {};
}

const AssetImporterDescriptor* AssetImporterRegistry::Find(std::string_view id) const noexcept
{
    const auto it = m_Entries.find(std::string(id));
    return it == m_Entries.end() ? nullptr : &it->second;
}

std::vector<std::string> AssetImporterRegistry::Candidates(const std::filesystem::path& input,
    std::span<const std::byte> leadingBytes) const
{
    std::vector<std::string> result;
    const auto extension = LowerExtension(input);
    for (const auto& [id, descriptor] : m_Entries)
    {
        const bool extensionMatches = descriptor.extensions.empty()
            || std::find(descriptor.extensions.begin(), descriptor.extensions.end(), extension) != descriptor.extensions.end();
        if (extensionMatches && (!descriptor.probe || descriptor.probe(leadingBytes))) result.push_back(id);
    }
    std::sort(result.begin(), result.end());
    return result;
}

AssetImportTask StartAssetImport(AssetImportRequest request, AssetImporterDescriptor importer,
    std::filesystem::path projectRoot, ProjectAssets::Limits limits)
{
    auto cancelled = std::make_shared<std::atomic_bool>(false);
    auto result = std::async(std::launch::async, [request = std::move(request), importer = std::move(importer),
        projectRoot = std::move(projectRoot), limits, cancelled]() mutable {
        try { return PrepareImport(std::move(request), std::move(importer), projectRoot, limits, cancelled); }
        catch (const std::exception& exception) { return std::expected<PreparedAssetImport, std::string>(std::unexpected(std::string("asset importer failed: ") + exception.what())); }
        catch (...) { return std::expected<PreparedAssetImport, std::string>(std::unexpected(std::string("asset importer failed with an unknown exception"))); }
    });
    return {std::move(cancelled), std::move(result)};
}

EntityId SubmitAssetImport(World& editorWorld, AssetImportRequest request)
{
    const auto entity = editorWorld.CreateEntity();
    editorWorld.AddComponent<AssetImportRequestComponent>(entity, AssetImportRequestComponent{std::move(request), false});
    editorWorld.AddComponent<AssetImportResultComponent>(entity);
    return entity;
}

void CancelAssetImport(World& editorWorld, EntityId requestEntity)
{
    if (!editorWorld.IsValid(requestEntity) || !editorWorld.HasComponent<AssetImportRequestComponent>(requestEntity)) return;
    editorWorld.GetComponent<AssetImportRequestComponent>(requestEntity).cancelled = true;
    if (editorWorld.HasComponent<AssetImportTaskComponent>(requestEntity))
        editorWorld.GetComponent<AssetImportTaskComponent>(requestEntity).task.Cancel();
    else if (editorWorld.HasComponent<AssetImportResultComponent>(requestEntity))
    {
        auto& result = editorWorld.GetComponent<AssetImportResultComponent>(requestEntity);
        if (result.state == AssetImportTaskState::Queued) result.state = AssetImportTaskState::Cancelled;
    }
}

void CommitReadyImports(World& editorWorld)
{
    if (editorWorld.IsDispatching()) return;
    EntityId projectEntity = entt::null;
    EntityId servicesEntity = entt::null;
    for (const auto entity : editorWorld.Select<ProjectStateComponent>()) { projectEntity = entity; break; }
    for (const auto entity : editorWorld.Select<EditorServicesComponent>()) { servicesEntity = entity; break; }
    std::vector<EntityId> requests;
    for (const auto entity : editorWorld.Select<AssetImportRequestComponent, AssetImportResultComponent>()) requests.push_back(entity);
    if (requests.empty()) return;
    for (const auto entity : requests)
    {
        auto& request = editorWorld.GetComponent<AssetImportRequestComponent>(entity);
        auto& result = editorWorld.GetComponent<AssetImportResultComponent>(entity);
        if (result.state == AssetImportTaskState::Committed || result.state == AssetImportTaskState::Rejected
            || result.state == AssetImportTaskState::Cancelled) continue;
        if (request.cancelled)
        {
            if (editorWorld.HasComponent<AssetImportTaskComponent>(entity))
            {
                auto& task = editorWorld.GetComponent<AssetImportTaskComponent>(entity).task;
                task.Cancel();
                if (task.result.wait_for(std::chrono::seconds(0)) != std::future_status::ready) continue;
                auto prepared = task.result.get();
                if (prepared && !prepared->stagingDirectory.empty())
                {
                    std::error_code ignored;
                    std::filesystem::remove_all(prepared->stagingDirectory, ignored);
                }
                editorWorld.RemoveComponent<AssetImportTaskComponent>(entity);
            }
            result.state = AssetImportTaskState::Cancelled;
            result.error = "asset import was cancelled";
            continue;
        }
        if (projectEntity == entt::null || servicesEntity == entt::null)
        {
            result.state = AssetImportTaskState::Rejected;
            result.error = "project import services are not registered";
            continue;
        }
        auto& project = editorWorld.GetComponent<ProjectStateComponent>(projectEntity);
        auto& services = editorWorld.GetComponent<EditorServicesComponent>(servicesEntity);
        if (result.state == AssetImportTaskState::Queued)
        {
            if (!project.open || !project.writable || !project.writeLock || !project.catalog || !project.importers
                || !services.runtime)
            {
                result.state = AssetImportTaskState::Rejected;
                result.error = "project is closed, read-only, or missing its import services";
                continue;
            }
            if (project.projectId != request.request.projectId || project.sessionGeneration != request.request.sessionGeneration)
            {
                result.state = AssetImportTaskState::Rejected;
                result.error = "import request belongs to a stale project session";
                continue;
            }
            bool playLocked = false;
            for (const auto playEntity : editorWorld.Select<PlaySessionComponent>())
                if (editorWorld.GetComponent<PlaySessionComponent>(playEntity).state != PlayState::Stopped) playLocked = true;
            if (playLocked)
            {
                result.state = AssetImportTaskState::Rejected;
                result.error = "asset imports are locked while Play is active";
                continue;
            }
            const auto* importer = project.importers->Find(request.request.importerId);
            if (!importer)
            {
                result.state = AssetImportTaskState::Rejected;
                result.error = "requested importer is not registered";
                continue;
            }
            try
            {
                auto task = StartAssetImport(request.request, *importer, project.projectRoot);
                editorWorld.AddComponent<AssetImportTaskComponent>(entity, AssetImportTaskComponent{std::move(task)});
                result.state = AssetImportTaskState::Running;
                result.error.clear();
            }
            catch (const std::exception& exception)
            {
                result.state = AssetImportTaskState::Rejected;
                result.error = std::string("could not start import worker: ") + exception.what();
            }
            continue;
        }
        if (result.state != AssetImportTaskState::Running || !editorWorld.HasComponent<AssetImportTaskComponent>(entity)) continue;
        auto& task = editorWorld.GetComponent<AssetImportTaskComponent>(entity).task;
        if (task.result.wait_for(std::chrono::seconds(0)) != std::future_status::ready) continue;
        auto prepared = task.result.get();
        editorWorld.RemoveComponent<AssetImportTaskComponent>(entity);
        if (!prepared)
        {
            result.state = request.cancelled ? AssetImportTaskState::Cancelled : AssetImportTaskState::Rejected;
            result.error = prepared.error();
            continue;
        }
        bool playLocked = false;
        for (const auto playEntity : editorWorld.Select<PlaySessionComponent>())
            if (editorWorld.GetComponent<PlaySessionComponent>(playEntity).state != PlayState::Stopped) playLocked = true;
        AssetImportCommitContext context{project.projectId, project.sessionGeneration, playLocked,
            project.writable && static_cast<bool>(project.writeLock)};
        const auto catalog = project.catalog;
        const auto assetId = prepared->record.id;
        if (!catalog || !services.runtime)
        {
            result.state = AssetImportTaskState::Rejected;
            result.error = "project catalog or Runtime registry was closed before import commit";
            continue;
        }
        auto committed = CommitAssetImport(std::move(*prepared), context, *catalog, *services.runtime,
            project.projectRoot);
        if (!committed)
        {
            result.state = request.cancelled ? AssetImportTaskState::Cancelled : AssetImportTaskState::Rejected;
            result.error = committed.error().message;
            continue;
        }
        project.catalog = *committed;
        services.catalog = *committed;
        const auto* record = (*committed)->Find(assetId);
        result.state = AssetImportTaskState::Committed;
        result.error.clear();
        if (record)
        {
            result.asset = ProjectAssets::ProjectAssetRef{(*committed)->Project(), record->id};
            result.revision = record->revision;
        }
    }
}

ProjectAssets::Result<std::shared_ptr<const ProjectAssets::CatalogSnapshot>> CommitAssetImport(
    PreparedAssetImport prepared, const AssetImportCommitContext& context,
    const ProjectAssets::CatalogSnapshot& currentCatalog, const GameFeatures::RuntimeRegistry& runtime,
    const std::filesystem::path& projectRoot, Storage::AtomicFileOperations* fileOperations,
    ProjectAssets::Limits limits)
{
    using namespace ProjectAssets;
    struct StagingCleanup
    {
        std::filesystem::path path;
        ~StagingCleanup() { if (!path.empty()) { std::error_code ignored; std::filesystem::remove_all(path, ignored); } }
    } cleanup{prepared.stagingDirectory};
    if (prepared.cancelled && prepared.cancelled->load(std::memory_order_relaxed))
        return std::unexpected(ImportError(ErrorCode::Conflict, "asset import was cancelled before commit"));
    if (!context.writable) return std::unexpected(ImportError(ErrorCode::Frozen, "project is read-only"));
    if (context.playLocked) return std::unexpected(ImportError(ErrorCode::Frozen, "asset writes are locked while Play is active"));
    if (context.currentProjectId != prepared.request.projectId || currentCatalog.Project() != context.currentProjectId)
        return std::unexpected(ImportError(ErrorCode::WrongProject, "import result belongs to another project"));
    if (context.currentSessionGeneration != prepared.request.sessionGeneration)
        return std::unexpected(ImportError(ErrorCode::Conflict, "import result belongs to a stale project session"));
    if (currentCatalog.Generation() != prepared.request.catalogGeneration)
        return std::unexpected(ImportError(ErrorCode::Conflict, "catalog changed while the import was running"));
    if (prepared.stagingDirectory.empty() || !std::filesystem::is_directory(prepared.stagingDirectory))
        return std::unexpected(ImportError(ErrorCode::IoError, "import staging revision is missing"));
    const auto* type = runtime.AssetTypes().Find(prepared.record.type);
    if (!type || type->ownerFeature != prepared.record.ownerFeature || type->formatVersion != prepared.record.formatVersion)
        return std::unexpected(ImportError(ErrorCode::WrongType, "prepared asset type differs from Runtime registration"));
    const auto* previous = currentCatalog.Find(prepared.record.id);
    if (prepared.request.existingAsset)
    {
        if (!previous || previous->revision != prepared.request.baseRevision)
            return std::unexpected(ImportError(ErrorCode::Conflict, "asset revision changed while reimport was running"));
        if (previous->type != prepared.record.type || previous->ownerFeature != prepared.record.ownerFeature)
            return std::unexpected(ImportError(ErrorCode::WrongType, "reimport cannot change the asset's Runtime type or owner"));
        if (prepared.record.revision != prepared.request.baseRevision + 1)
            return std::unexpected(ImportError(ErrorCode::Conflict, "prepared reimport revision is invalid"));
    }
    else if (previous || prepared.request.baseRevision != 0 || prepared.record.revision != 1)
        return std::unexpected(ImportError(ErrorCode::Conflict, "new asset import collides with an existing AssetId"));
    if (prepared.request.catalogGeneration == std::numeric_limits<std::uint64_t>::max())
        return std::unexpected(ImportError(ErrorCode::LimitExceeded, "catalog generation is exhausted"));

    auto root = std::filesystem::weakly_canonical(projectRoot);
    const auto stage = std::filesystem::weakly_canonical(prepared.stagingDirectory);
    if (!IsWithin(root / ".aether" / "staging", stage))
        return std::unexpected(ImportError(ErrorCode::UnsafePath, "staging revision is outside the project staging directory"));
    const auto assetRoot = root / "Assets" / "Data" / prepared.record.id.ToString();
    std::error_code ec;
    std::filesystem::create_directories(assetRoot, ec);
    if (ec) return std::unexpected(ImportError(ErrorCode::IoError, "could not create asset revision directory: " + ec.message()));
    std::uint64_t revision = prepared.record.revision;
    while (std::filesystem::exists(assetRoot / std::to_string(revision), ec))
    {
        if (ec || revision == std::numeric_limits<std::uint64_t>::max())
            return std::unexpected(ImportError(ErrorCode::LimitExceeded, "no available asset revision number"));
        ++revision;
    }
    prepared.record.revision = revision;
    prepared.record.artifactRoot = "Assets/Data/" + prepared.record.id.ToString() + "/" + std::to_string(revision);
    const auto revisionRoot = root / prepared.record.artifactRoot;
    std::filesystem::rename(stage, revisionRoot, ec);
    if (ec) return std::unexpected(ImportError(ErrorCode::IoError, "could not publish staged asset revision: " + ec.message()));
    cleanup.path.clear(); // A catalog failure leaves an invisible orphan revision, which startup ignores.

    const auto entry = std::filesystem::weakly_canonical(revisionRoot / prepared.record.entryPoint, ec);
    if (ec || !IsWithin(std::filesystem::weakly_canonical(revisionRoot, ec), entry))
        return std::unexpected(ImportError(ErrorCode::UnsafePath, "asset entry point resolves outside the published revision"));
    if (type->loadMetadata)
    {
        auto metadata = type->loadMetadata(entry);
        if (!metadata) return std::unexpected(metadata.error());
    }
    if (type->enumerateDependencies)
    {
        auto enumerated = type->enumerateDependencies(entry);
        if (!enumerated) return std::unexpected(enumerated.error());
        auto expected = prepared.record.dependencies;
        SortReferences(*enumerated);
        SortReferences(expected);
        if (*enumerated != expected)
            return std::unexpected(ImportError(ErrorCode::InvalidFormat, "asset dependency list differs from importer output"));
    }

    CatalogData nextData{currentCatalog.Project(), currentCatalog.Generation() + 1, currentCatalog.Records()};
    if (previous)
    {
        const auto position = std::find_if(nextData.records.begin(), nextData.records.end(), [&](const auto& item) { return item.id == prepared.record.id; });
        if (position == nextData.records.end()) return std::unexpected(ImportError(ErrorCode::Conflict, "asset disappeared from current catalog"));
        *position = prepared.record;
    }
    else nextData.records.push_back(prepared.record);
    auto snapshot = CatalogSnapshot::Create(std::move(nextData), runtime.AssetTypes());
    if (!snapshot) return std::unexpected(snapshot.error());
    auto closure = ValidateReferenceClosure(**snapshot, {currentCatalog.Project(), prepared.record.id}, root,
                                           runtime.AssetTypes(), "import.dependencies");
    if (!closure) return std::unexpected(closure.error());
    auto encoded = EncodeCatalog(**snapshot, limits);
    if (!encoded) return std::unexpected(encoded.error());
    const auto catalogText = encoded->dump(2);
    if (catalogText.size() > limits.maxManifestBytes)
        return std::unexpected(ImportError(ErrorCode::LimitExceeded, "encoded catalog exceeds byte limit"));
    const auto catalogBytes = std::as_bytes(std::span(catalogText.data(), catalogText.size()));
    auto committed = Storage::AtomicReplaceFile(root / "Assets" / "catalog.json", catalogBytes, fileOperations);
    if (!committed) return std::unexpected(ImportError(ErrorCode::IoError, committed.error(), "Assets/catalog.json"));
    return *snapshot;
}

ProjectAssets::Result<std::shared_ptr<const ProjectAssets::CatalogSnapshot>> CommitAssetRename(
    const AssetRenameRequest& request, const AssetImportCommitContext& context,
    const ProjectAssets::CatalogSnapshot& currentCatalog, const GameFeatures::RuntimeRegistry& runtime,
    const std::filesystem::path& projectRoot, Storage::AtomicFileOperations* fileOperations,
    ProjectAssets::Limits limits)
{
    using namespace ProjectAssets;
    if (!context.writable) return std::unexpected(ImportError(ErrorCode::Frozen, "project is read-only"));
    if (context.playLocked) return std::unexpected(ImportError(ErrorCode::Frozen, "asset writes are locked while Play is active"));
    if (!request.projectId.IsValid() || context.currentProjectId != request.projectId
        || currentCatalog.Project() != request.projectId)
        return std::unexpected(ImportError(ErrorCode::WrongProject, "rename request belongs to another project"));
    if (request.sessionGeneration == 0 || context.currentSessionGeneration != request.sessionGeneration)
        return std::unexpected(ImportError(ErrorCode::Conflict, "rename request belongs to a stale project session"));
    if (request.catalogGeneration != currentCatalog.Generation())
        return std::unexpected(ImportError(ErrorCode::Conflict, "catalog changed before asset rename"));
    if (!request.assetId.IsValid())
        return std::unexpected(ImportError(ErrorCode::InvalidArgument, "rename request has an invalid AssetId"));
    if (!SafeDisplayPath(request.displayPath))
        return std::unexpected(ImportError(ErrorCode::UnsafePath, "display path is empty, non-canonical, or escapes the logical asset namespace", "displayPath"));
    const auto* record = currentCatalog.Find(request.assetId);
    if (!record) return std::unexpected(ImportError(ErrorCode::MissingAsset, "asset to rename is absent from the catalog"));
    for (const auto& other : currentCatalog.Records())
        if (other.id != request.assetId && other.displayPath == request.displayPath)
            return std::unexpected(ImportError(ErrorCode::Conflict, "another asset already uses this display path", "displayPath"));
    if (record->displayPath == request.displayPath)
        return CatalogSnapshot::Create({currentCatalog.Project(), currentCatalog.Generation(), currentCatalog.Records()},
            runtime.AssetTypes());
    if (currentCatalog.Generation() == std::numeric_limits<std::uint64_t>::max())
        return std::unexpected(ImportError(ErrorCode::LimitExceeded, "catalog generation is exhausted"));

    CatalogData nextData{currentCatalog.Project(), currentCatalog.Generation() + 1, currentCatalog.Records()};
    const auto position = std::find_if(nextData.records.begin(), nextData.records.end(), [&](const auto& item) {
        return item.id == request.assetId;
    });
    if (position == nextData.records.end())
        return std::unexpected(ImportError(ErrorCode::Conflict, "asset disappeared from current catalog"));
    position->displayPath = request.displayPath;
    auto snapshot = CatalogSnapshot::Create(std::move(nextData), runtime.AssetTypes());
    if (!snapshot) return std::unexpected(snapshot.error());
    auto encoded = EncodeCatalog(**snapshot, limits);
    if (!encoded) return std::unexpected(encoded.error());
    const auto catalogText = encoded->dump(2);
    const auto catalogBytes = std::as_bytes(std::span(catalogText.data(), catalogText.size()));
    std::error_code ec;
    const auto root = std::filesystem::weakly_canonical(projectRoot, ec);
    if (ec) return std::unexpected(ImportError(ErrorCode::IoError, "could not resolve project root"));
    const auto catalogPath = std::filesystem::weakly_canonical(root / "Assets" / "catalog.json", ec);
    if (ec || !IsWithin(root, catalogPath))
        return std::unexpected(ImportError(ErrorCode::UnsafePath, "asset catalog resolves outside project root"));
    auto committed = Storage::AtomicReplaceFile(catalogPath, catalogBytes, fileOperations);
    if (!committed) return std::unexpected(ImportError(ErrorCode::IoError, committed.error(), "Assets/catalog.json"));
    return *snapshot;
}
}
