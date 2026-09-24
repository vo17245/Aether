#include <World/Serialization/WorldArchive.h>

#include <algorithm>
#include <array>
#include <atomic>
#include <cerrno>
#include <chrono>
#include <cmath>
#include <fstream>
#include <iterator>
#include <limits>
#include <random>
#include <set>
#include <stdexcept>
#include <system_error>

#if defined(__APPLE__)
#include <stdio.h>
#include <sys/attr.h>
#elif defined(__linux__)
#include <fcntl.h>
#include <sys/syscall.h>
#include <unistd.h>
#endif

namespace Aether::Serialization
{
namespace
{
ArchiveError MakeError(ArchiveErrorCode code, std::string message, std::filesystem::path path = {}, std::string pointer = {})
{
    ArchiveError error;
    error.code = code;
    error.message = std::move(message);
    error.path = std::move(path);
    error.jsonPointer = std::move(pointer);
    return error;
}

bool IsSafeRelative(std::string_view value)
{
    if (value.empty() || value.find('\0') != std::string_view::npos || value.find('\\') != std::string_view::npos
        || value.find(':') != std::string_view::npos || value.front() == '/') return false;
    const std::filesystem::path path{std::string(value)};
    if (path.empty() || path.is_absolute() || path.has_root_name() || path.has_root_directory()) return false;
    for (const auto& part : path)
        if (part.empty() || part == "." || part == "..") return false;
    return path.generic_string() == value;
}

Result<std::filesystem::path> ResolveSourcePath(const std::filesystem::path& source,
                                                const std::filesystem::path& sourceRoot)
{
    if (source.empty())
        return std::unexpected(MakeError(ArchiveErrorCode::InvalidArgument, "source asset path cannot be empty"));
    if (source.is_absolute()) return source;
    if (sourceRoot.empty())
        return std::unexpected(MakeError(ArchiveErrorCode::InvalidArgument,
                                         "relative source asset path requires an explicit source root", source));
    const auto relative = source.generic_string();
    if (!IsSafeRelative(relative))
        return std::unexpected(MakeError(ArchiveErrorCode::UnsafePath,
                                         "relative source asset path must be canonical and remain under its source root", source));
    std::error_code ec;
    const auto canonicalRoot = std::filesystem::canonical(sourceRoot, ec);
    if (ec)
        return std::unexpected(MakeError(ArchiveErrorCode::InvalidArgument, "source root is not accessible: " + ec.message(), sourceRoot));
    const auto canonicalSource = std::filesystem::canonical(canonicalRoot / source, ec);
    if (ec)
        return std::unexpected(MakeError(ArchiveErrorCode::MissingAsset, "relative source asset does not exist: " + ec.message(), canonicalRoot / source));
    const auto contained = canonicalSource.lexically_relative(canonicalRoot);
    if (contained.empty() || contained.is_absolute() || *contained.begin() == "..")
        return std::unexpected(MakeError(ArchiveErrorCode::UnsafePath, "relative source asset escapes its source root", canonicalSource));
    return canonicalSource;
}

bool HasOnlyFiniteNumbers(const Json& value)
{
    if (value.is_number_float()) return std::isfinite(value.get<double>());
    if (value.is_array())
    {
        for (const auto& item : value) if (!HasOnlyFiniteNumbers(item)) return false;
    }
    else if (value.is_object())
    {
        for (auto it = value.begin(); it != value.end(); ++it) if (!HasOnlyFiniteNumbers(it.value())) return false;
    }
    return true;
}

class JsonDepthLimitExceeded final : public std::runtime_error
{
public: using std::runtime_error::runtime_error;
};

Result<void> CheckFileSize(const std::filesystem::path& path, std::uint64_t size, const ArchiveLimits& limits,
                           std::uint64_t& total)
{
    if (size > limits.maxSingleAssetBytes)
        return std::unexpected(MakeError(ArchiveErrorCode::ResourceLimitExceeded, "asset exceeds per-file byte limit", path));
    if (total > limits.maxTotalAssetBytes || size > limits.maxTotalAssetBytes - total)
        return std::unexpected(MakeError(ArchiveErrorCode::ResourceLimitExceeded, "assets exceed total byte limit", path));
    total += size;
    return {};
}

Result<void> CopyBounded(const std::filesystem::path& source, const std::filesystem::path& destination,
                         const ArchiveLimits& limits, std::uint64_t& total)
{
    std::error_code ec;
    const auto status = std::filesystem::symlink_status(source, ec);
    if (ec || !std::filesystem::is_regular_file(status))
        return std::unexpected(MakeError(ArchiveErrorCode::MissingAsset, "source asset is not a regular file", source));
    const auto size = std::filesystem::file_size(source, ec);
    if (ec) return std::unexpected(MakeError(ArchiveErrorCode::IoError, "cannot read source asset size: " + ec.message(), source));
    auto checked = CheckFileSize(source, size, limits, total);
    if (!checked) return checked;
    if (!std::filesystem::create_directories(destination.parent_path(), ec) && ec)
        return std::unexpected(MakeError(ArchiveErrorCode::IoError, "cannot create asset directory: " + ec.message(), destination.parent_path()));
    if (std::filesystem::exists(destination, ec) || ec)
        return std::unexpected(MakeError(ArchiveErrorCode::IoError, "asset destination already exists", destination));
    if (!std::filesystem::copy_file(source, destination, std::filesystem::copy_options::none, ec) || ec)
        return std::unexpected(MakeError(ArchiveErrorCode::IoError, "cannot copy asset: " + ec.message(), destination));
    return {};
}

Result<std::filesystem::path> ResolveSafeFile(const std::filesystem::path& root, std::string_view relative)
{
    if (!IsSafeRelative(relative) || !relative.starts_with("assets/"))
        return std::unexpected(MakeError(ArchiveErrorCode::UnsafePath, "asset path is not a canonical relative assets path", root / std::string(relative)));
    std::filesystem::path current = root;
    std::error_code ec;
    auto status = std::filesystem::symlink_status(root, ec);
    if (ec || std::filesystem::is_symlink(status) || !std::filesystem::is_directory(status))
        return std::unexpected(MakeError(ArchiveErrorCode::UnsafePath, "package root must be a real directory", root));
    for (const auto& part : std::filesystem::path(std::string(relative)))
    {
        current /= part;
        status = std::filesystem::symlink_status(current, ec);
        if (ec == std::errc::no_such_file_or_directory)
            return std::unexpected(MakeError(ArchiveErrorCode::MissingAsset, "asset path does not exist", current));
        if (ec)
            return std::unexpected(MakeError(ArchiveErrorCode::IoError, "cannot inspect asset path: " + ec.message(), current));
        if (std::filesystem::is_symlink(status))
            return std::unexpected(MakeError(ArchiveErrorCode::UnsafePath, "asset path contains a symlink", current));
    }
    if (!std::filesystem::is_regular_file(status))
        return std::unexpected(MakeError(ArchiveErrorCode::MissingAsset, "asset is not a regular file", current));
    const auto canonical = std::filesystem::canonical(current, ec);
    if (ec) return std::unexpected(MakeError(ArchiveErrorCode::IoError, "cannot canonicalize asset: " + ec.message(), current));
    const auto canonicalRoot = std::filesystem::canonical(root, ec);
    if (ec) return std::unexpected(MakeError(ArchiveErrorCode::IoError, "cannot canonicalize package root: " + ec.message(), root));
    const auto rel = canonical.lexically_relative(canonicalRoot);
    if (rel.empty() || rel.is_absolute() || *rel.begin() == "..")
        return std::unexpected(MakeError(ArchiveErrorCode::UnsafePath, "asset resolves outside package root", current));
    return canonical;
}

std::string AssetPath(const std::filesystem::path& path)
{
    return path.generic_string();
}

bool ExclusiveRename(const std::filesystem::path& source, const std::filesystem::path& target, int& error)
{
#if defined(__APPLE__)
    if (::renamex_np(source.c_str(), target.c_str(), RENAME_EXCL) == 0) return true;
    error = errno;
    return false;
#elif defined(__linux__) && defined(SYS_renameat2)
    if (::syscall(SYS_renameat2, AT_FDCWD, source.c_str(), AT_FDCWD, target.c_str(), 1u) == 0) return true;
    error = errno;
    return false;
#else
    (void)source; (void)target;
    error = ENOTSUP;
    return false;
#endif
}

struct StagingDirectory
{
    std::filesystem::path path;
    bool published = false;
    ~StagingDirectory()
    {
        if (!published && !path.empty())
        {
            std::error_code ignored;
            std::filesystem::remove_all(path, ignored);
        }
    }
};

Result<std::uint64_t> ReadUnsigned(const Json& value, std::string pointer)
{
    if (value.is_number_unsigned()) return value.get<std::uint64_t>();
    if (value.is_number_integer())
    {
        auto n = value.get<std::int64_t>();
        if (n >= 0) return static_cast<std::uint64_t>(n);
    }
    return std::unexpected(MakeError(ArchiveErrorCode::InvalidFormat, "expected a non-negative integer", {}, std::move(pointer)));
}

Result<Json> ParseChecked(const std::string& source, const ArchiveLimits& limits)
{
    std::vector<std::set<std::string>> objectKeys;
    auto callback = [&](int depth, Json::parse_event_t event, Json& parsed) {
        if (depth > static_cast<int>(limits.maxJsonDepth))
            throw JsonDepthLimitExceeded("JSON nesting depth exceeds archive limit");
        if (event == Json::parse_event_t::object_start) objectKeys.emplace_back();
        else if (event == Json::parse_event_t::object_end)
        {
            if (!objectKeys.empty()) objectKeys.pop_back();
        }
        else if (event == Json::parse_event_t::key)
        {
            if (objectKeys.empty() || !objectKeys.back().emplace(parsed.get<std::string>()).second)
                throw std::runtime_error("duplicate JSON object key");
        }
        return true;
    };
    try { return Json::parse(source, callback); }
    catch (const std::bad_alloc&) { return std::unexpected(MakeError(ArchiveErrorCode::AllocationFailure, "allocation failed while parsing world manifest")); }
    catch (const JsonDepthLimitExceeded& exception) { return std::unexpected(MakeError(ArchiveErrorCode::ResourceLimitExceeded, exception.what())); }
    catch (const std::exception& exception) { return std::unexpected(MakeError(ArchiveErrorCode::InvalidFormat, exception.what())); }
}
} // namespace

const AssetEntry* AssetCatalog::Find(std::string_view path) const noexcept
{
    const auto it = m_ByPath.find(std::string(path));
    return it == m_ByPath.end() ? nullptr : &m_Entries[it->second];
}

LoadedWorldDirectory::LoadedWorldDirectory(std::unique_ptr<World> loadedWorld, Json loadedApplication,
                                           std::filesystem::path loadedPackageRoot,
                                           std::shared_ptr<const AssetCatalog> loadedAssets)
    : world(std::move(loadedWorld)), application(std::move(loadedApplication)),
      packageRoot(std::move(loadedPackageRoot)), assets(std::move(loadedAssets)) {}
LoadedWorldDirectory::~LoadedWorldDirectory() = default;
LoadedWorldDirectory::LoadedWorldDirectory(LoadedWorldDirectory&&) noexcept = default;
LoadedWorldDirectory& LoadedWorldDirectory::operator=(LoadedWorldDirectory&&) noexcept = default;

Result<void> SaveContext::RegisterAssetFile(std::string path, std::string kind, const std::filesystem::path& file)
{
    if (!IsSafeRelative(path) || !path.starts_with("assets/"))
        return std::unexpected(MakeError(ArchiveErrorCode::UnsafePath, "generated asset path is invalid", file));
    const auto expectedFile = (m_PackageRoot / std::filesystem::path(path)).lexically_normal();
    if (m_PackageRoot.empty() || expectedFile != file.lexically_normal())
        return std::unexpected(MakeError(ArchiveErrorCode::UnsafePath, "generated asset is outside its registered package path", file));
    if (m_Assets.size() >= m_Limits.maxAssetFiles)
        return std::unexpected(MakeError(ArchiveErrorCode::ResourceLimitExceeded, "asset file count exceeds archive limit", file));
    if (m_AssetByPath.contains(path))
        return std::unexpected(MakeError(ArchiveErrorCode::InvalidArgument, "asset path is already registered", file));
    std::error_code ec;
    auto current = m_PackageRoot;
    const auto relativeFile = std::filesystem::path(path);
    for (auto iter = relativeFile.begin(); iter != relativeFile.end(); ++iter)
    {
        current /= *iter;
        const auto status = std::filesystem::symlink_status(current, ec);
        if (ec || std::filesystem::is_symlink(status))
            return std::unexpected(MakeError(ArchiveErrorCode::UnsafePath, "generated asset path contains a symlink or missing entry", current));
        const bool last = std::next(iter) == relativeFile.end();
        if (last ? !std::filesystem::is_regular_file(status) : !std::filesystem::is_directory(status))
            return std::unexpected(MakeError(ArchiveErrorCode::IoError, "generated asset path has an invalid file type", current));
    }
    const auto size = std::filesystem::file_size(file, ec);
    if (ec) return std::unexpected(MakeError(ArchiveErrorCode::IoError, "cannot read generated asset size: " + ec.message(), file));
    auto checked = CheckFileSize(file, size, m_Limits, m_AssetBytes);
    if (!checked) return checked;
    m_AssetByPath.emplace(path, size);
    AssetEntry entry;
    entry.path = std::move(path);
    entry.kind = std::move(kind);
    entry.byteSize = size;
    m_Assets.push_back(std::move(entry));
    return {};
}

Result<AssetRef> SaveContext::CopyFile(const std::filesystem::path& source, std::string kind)
{
    if (kind.empty()) return std::unexpected(MakeError(ArchiveErrorCode::InvalidArgument, "asset kind cannot be empty", source));
    auto resolvedSource = ResolveSourcePath(source, m_SourceRoot);
    if (!resolvedSource) return std::unexpected(resolvedSource.error());
    const auto extension = resolvedSource->extension().string();
    const auto rel = std::filesystem::path("assets/files") / (std::to_string(m_NextGenerated++) + extension);
    const auto path = AssetPath(rel);
    std::uint64_t unusedTotal = 0;
    auto copied = CopyBounded(*resolvedSource, m_PackageRoot / rel, m_Limits, unusedTotal);
    if (!copied) return std::unexpected(copied.error());
    auto added = RegisterAssetFile(path, kind, m_PackageRoot / rel);
    if (!added) return std::unexpected(added.error());
    return AssetRef{path, std::move(kind)};
}

Result<AssetRef> SaveContext::WriteAsset(std::string kind, std::string extension,
                                         const std::function<Result<void>(const std::filesystem::path&)>& writer)
{
    if (kind.empty() || !writer)
        return std::unexpected(MakeError(ArchiveErrorCode::InvalidArgument, "asset kind and writer are required"));
    if (extension.empty() || extension.front() != '.' || extension.find('/') != std::string::npos
        || extension.find('\\') != std::string::npos)
        return std::unexpected(MakeError(ArchiveErrorCode::InvalidArgument, "asset extension must be a simple dotted extension"));
    const auto rel = std::filesystem::path("assets/generated") / (std::to_string(m_NextGenerated++) + extension);
    const auto path = AssetPath(rel);
    const auto output = m_PackageRoot / rel;
    std::error_code ec;
    std::filesystem::create_directories(output.parent_path(), ec);
    if (ec) return std::unexpected(MakeError(ArchiveErrorCode::IoError, "cannot create generated asset directory: " + ec.message(), output.parent_path()));
    auto result = writer(output);
    if (!result)
    {
        std::filesystem::remove(output, ec);
        auto error = result.error();
        if (error.path.empty()) error.path = output;
        return std::unexpected(std::move(error));
    }
    auto added = RegisterAssetFile(path, kind, output);
    if (!added)
    {
        std::filesystem::remove(output, ec);
        return std::unexpected(added.error());
    }
    return AssetRef{path, std::move(kind)};
}

std::string SaveContext::CreateAssetGroup()
{
    return (std::filesystem::path("assets/models") / (std::to_string(m_NextGroup++))).generic_string();
}

Result<AssetRef> SaveContext::CopyGroupFile(std::string_view group, std::string_view relativePath,
                                            const std::filesystem::path& source, std::string kind)
{
    if (kind.empty()) return std::unexpected(MakeError(ArchiveErrorCode::InvalidArgument, "asset kind cannot be empty", source));
    auto resolvedSource = ResolveSourcePath(source, m_SourceRoot);
    if (!resolvedSource) return std::unexpected(resolvedSource.error());
    if (!IsSafeRelative(group) || !group.starts_with("assets/models/") || !IsSafeRelative(relativePath))
        return std::unexpected(MakeError(ArchiveErrorCode::UnsafePath, "asset group path is invalid"));
    const std::filesystem::path rel = std::filesystem::path(std::string(group)) / std::filesystem::path(std::string(relativePath));
    const auto path = AssetPath(rel);
    if (m_AssetByPath.contains(path))
        return std::unexpected(MakeError(ArchiveErrorCode::InvalidArgument, "asset group destination is already registered", m_PackageRoot / rel));
    std::uint64_t unusedTotal = 0;
    auto copied = CopyBounded(*resolvedSource, m_PackageRoot / rel, m_Limits, unusedTotal);
    if (!copied) return std::unexpected(copied.error());
    auto added = RegisterAssetFile(path, kind, m_PackageRoot / rel);
    if (!added) return std::unexpected(added.error());
    return AssetRef{path, std::move(kind)};
}

Result<std::filesystem::path> LoadContext::ResolveAsset(const AssetRef& ref, std::string_view expectedKind) const
{
    if (!m_Assets) return std::unexpected(MakeError(ArchiveErrorCode::InvalidArgument, "asset catalog is not initialized"));
    const auto* entry = m_Assets->Find(ref.path);
    if (!entry) return std::unexpected(MakeError(ArchiveErrorCode::MissingAsset, "asset is not registered in the manifest", m_PackageRoot / ref.path));
    if (ref.kind != entry->kind || entry->kind != expectedKind)
        return std::unexpected(MakeError(ArchiveErrorCode::InvalidFormat, "asset kind does not match its reference", m_PackageRoot / ref.path));
    return ResolveSafeFile(m_PackageRoot, ref.path);
}

Result<std::filesystem::path> LoadContext::ResolvePackagedPath(std::string_view relativePath, std::string_view expectedKind) const
{
    if (!m_Assets) return std::unexpected(MakeError(ArchiveErrorCode::InvalidArgument, "asset catalog is not initialized"));
    const auto* entry = m_Assets->Find(relativePath);
    if (!entry || entry->kind != expectedKind)
        return std::unexpected(MakeError(ArchiveErrorCode::MissingAsset, "packaged dependency is not registered with the expected kind", m_PackageRoot / std::string(relativePath)));
    return ResolveSafeFile(m_PackageRoot, relativePath);
}

Result<void> SaveWorldDirectory(const World& world, const ComponentCodecRegistry& codecs, const Json& application,
                                const std::filesystem::path& targetPath, const DirectorySaveOptions& options)
{
    if (targetPath.empty() || !application.is_object())
        return std::unexpected(MakeError(ArchiveErrorCode::InvalidArgument, "target path and object application data are required"));
    try
    {
        std::error_code ec;
        const auto absoluteTarget = std::filesystem::absolute(targetPath, ec).lexically_normal();
        if (ec) return std::unexpected(MakeError(ArchiveErrorCode::IoError, "cannot resolve target path: " + ec.message(), targetPath));
        const auto parent = absoluteTarget.parent_path();
        if (!std::filesystem::is_directory(parent, ec) || ec)
            return std::unexpected(MakeError(ArchiveErrorCode::InvalidArgument, "target parent must already exist", parent));
        const auto targetStatus = std::filesystem::symlink_status(absoluteTarget, ec);
        if (!ec && targetStatus.type() != std::filesystem::file_type::not_found)
            return std::unexpected(MakeError(ArchiveErrorCode::AlreadyExists, "Save As target already exists", absoluteTarget));
        if (ec && ec != std::errc::no_such_file_or_directory)
            return std::unexpected(MakeError(ArchiveErrorCode::IoError, "cannot inspect target: " + ec.message(), absoluteTarget));

        static std::atomic_uint64_t serial{1};
        StagingDirectory staging;
        for (int attempt = 0; attempt < 64; ++attempt)
        {
            const auto stamp = std::chrono::steady_clock::now().time_since_epoch().count();
            staging.path = parent / (absoluteTarget.filename().string() + ".tmp-" + std::to_string(stamp) + "-" + std::to_string(serial.fetch_add(1)));
            ec.clear();
            if (std::filesystem::create_directory(staging.path, ec)) break;
            if (ec && ec != std::errc::file_exists)
                return std::unexpected(MakeError(ArchiveErrorCode::IoError, "cannot create staging directory: " + ec.message(), staging.path));
            staging.path.clear();
        }
        if (staging.path.empty())
            return std::unexpected(MakeError(ArchiveErrorCode::IoError, "could not allocate a unique staging directory", parent));

        SaveContext context;
        context.m_PackageRoot = staging.path;
        ec.clear();
        context.m_SourceRoot = options.sourceRoot.empty() ? std::filesystem::path{} : std::filesystem::canonical(options.sourceRoot, ec);
        if (ec) return std::unexpected(MakeError(ArchiveErrorCode::InvalidArgument, "source root is not accessible: " + ec.message(), options.sourceRoot));
        context.m_Limits = options.limits;
        auto worldDocument = world.Serialize(codecs, context);
        if (!worldDocument) return std::unexpected(worldDocument.error());

        Json manifest = Json::object();
        manifest["format"] = "Aether.WorldDirectory";
        manifest["version"] = 1;
        manifest["application"] = application;
        manifest["assets"] = Json::array();
        std::sort(context.m_Assets.begin(), context.m_Assets.end(), [](const auto& left, const auto& right) { return left.path < right.path; });
        for (const auto& asset : context.m_Assets)
            manifest["assets"].push_back({{"path", asset.path}, {"kind", asset.kind}, {"byteSize", asset.byteSize}});
        manifest["entities"] = std::move((*worldDocument)["entities"]);
        if (!HasOnlyFiniteNumbers(manifest))
            return std::unexpected(MakeError(ArchiveErrorCode::InvalidFormat, "manifest contains a non-finite JSON number", staging.path / "world.json"));
        const auto content = manifest.dump(2) + "\n";
        if (content.size() > options.limits.maxManifestBytes)
            return std::unexpected(MakeError(ArchiveErrorCode::ResourceLimitExceeded, "manifest exceeds byte limit", staging.path / "world.json"));
        const auto manifestPath = staging.path / "world.json";
        {
            std::ofstream stream(manifestPath, std::ios::binary | std::ios::out | std::ios::trunc);
            if (!stream) return std::unexpected(MakeError(ArchiveErrorCode::IoError, "cannot create world manifest", manifestPath));
            stream.write(content.data(), static_cast<std::streamsize>(content.size()));
            stream.flush();
            if (!stream) return std::unexpected(MakeError(ArchiveErrorCode::IoError, "cannot write world manifest", manifestPath));
            stream.close();
            if (stream.fail()) return std::unexpected(MakeError(ArchiveErrorCode::IoError, "cannot close world manifest", manifestPath));
        }
        ec.clear();
        const auto size = std::filesystem::file_size(manifestPath, ec);
        if (ec || size != content.size()) return std::unexpected(MakeError(ArchiveErrorCode::IoError, "world manifest size verification failed", manifestPath));

        int renameError = 0;
        if (!ExclusiveRename(staging.path, absoluteTarget, renameError))
        {
            if (renameError == EEXIST || renameError == ENOTEMPTY)
                return std::unexpected(MakeError(ArchiveErrorCode::AlreadyExists, "Save As target appeared before publish", absoluteTarget));
            return std::unexpected(MakeError(ArchiveErrorCode::IoError, "atomic no-replace directory publish failed: " + std::error_code(renameError, std::generic_category()).message(), absoluteTarget));
        }
        staging.published = true;
        return {};
    }
    catch (const std::bad_alloc&)
    {
        return std::unexpected(MakeError(ArchiveErrorCode::AllocationFailure, "allocation failed while saving world directory", targetPath));
    }
    catch (const std::exception& exception)
    {
        return std::unexpected(MakeError(ArchiveErrorCode::IoError, exception.what(), targetPath));
    }
}

Result<LoadedWorldDirectory> LoadWorldDirectory(const std::filesystem::path& source,
                                                const ComponentCodecRegistry& codecs,
                                                const DirectoryLoadOptions& options)
{
    try
    {
        std::error_code ec;
        const auto packageRoot = std::filesystem::canonical(source, ec);
        if (ec || !std::filesystem::is_directory(packageRoot, ec) || ec)
            return std::unexpected(MakeError(ArchiveErrorCode::InvalidArgument, "package root is not a directory", source));
        const auto manifestPath = packageRoot / "world.json";
        const auto manifestStatus = std::filesystem::symlink_status(manifestPath, ec);
        if (ec || std::filesystem::is_symlink(manifestStatus) || !std::filesystem::is_regular_file(manifestStatus))
            return std::unexpected(MakeError(ArchiveErrorCode::MissingAsset, "world.json is missing or not a regular file", manifestPath));
        const auto manifestSize = std::filesystem::file_size(manifestPath, ec);
        if (ec) return std::unexpected(MakeError(ArchiveErrorCode::IoError, "cannot inspect world.json: " + ec.message(), manifestPath));
        if (manifestSize > options.limits.maxManifestBytes)
            return std::unexpected(MakeError(ArchiveErrorCode::ResourceLimitExceeded, "manifest exceeds byte limit", manifestPath));
        if (manifestSize > std::numeric_limits<std::size_t>::max()
            || manifestSize > static_cast<std::uint64_t>(std::numeric_limits<std::streamsize>::max()))
            return std::unexpected(MakeError(ArchiveErrorCode::ResourceLimitExceeded, "manifest does not fit addressable stream size", manifestPath));
        std::string content(static_cast<std::size_t>(manifestSize), '\0');
        {
            std::ifstream stream(manifestPath, std::ios::binary);
            if (!stream) return std::unexpected(MakeError(ArchiveErrorCode::IoError, "cannot open world.json", manifestPath));
            stream.read(content.data(), static_cast<std::streamsize>(content.size()));
            if (!stream || stream.peek() != std::char_traits<char>::eof())
                return std::unexpected(MakeError(ArchiveErrorCode::IoError, "cannot read complete world.json", manifestPath));
        }
        auto parsed = ParseChecked(content, options.limits);
        if (!parsed)
        {
            auto error = parsed.error(); error.path = manifestPath; return std::unexpected(std::move(error));
        }
        const auto& manifest = *parsed;
        if (!manifest.is_object() || !manifest.contains("format") || !manifest["format"].is_string()
            || manifest["format"] != "Aether.WorldDirectory")
            return std::unexpected(MakeError(ArchiveErrorCode::InvalidFormat, "invalid world directory format", manifestPath, "/format"));
        if (!manifest.contains("version"))
            return std::unexpected(MakeError(ArchiveErrorCode::InvalidFormat, "missing format version", manifestPath, "/version"));
        auto version = ReadUnsigned(manifest["version"], "/version");
        if (!version) return std::unexpected(version.error());
        if (*version != 1)
            return std::unexpected(MakeError(ArchiveErrorCode::UnsupportedVersion, "unsupported world directory version", manifestPath, "/version"));
        if (!manifest.contains("application") || !manifest["application"].is_object()
            || !manifest.contains("assets") || !manifest["assets"].is_array()
            || !manifest.contains("entities") || !manifest["entities"].is_array())
            return std::unexpected(MakeError(ArchiveErrorCode::InvalidFormat, "manifest requires application, assets, and entities", manifestPath));
        if (manifest["assets"].size() > options.limits.maxAssetFiles)
            return std::unexpected(MakeError(ArchiveErrorCode::ResourceLimitExceeded, "asset count exceeds archive limit", manifestPath, "/assets"));

        auto catalog = std::make_shared<AssetCatalog>();
        std::uint64_t totalBytes = 0;
        for (std::size_t index = 0; index < manifest["assets"].size(); ++index)
        {
            const auto& item = manifest["assets"][index];
            const auto pointer = "/assets/" + std::to_string(index);
            if (!item.is_object() || !item.contains("path") || !item["path"].is_string()
                || !item.contains("kind") || !item["kind"].is_string() || !item.contains("byteSize"))
                return std::unexpected(MakeError(ArchiveErrorCode::InvalidFormat, "asset entry requires path, kind, and byteSize", manifestPath, pointer));
            AssetEntry entry;
            entry.path = item["path"].get<std::string>();
            entry.kind = item["kind"].get<std::string>();
            auto byteSize = ReadUnsigned(item["byteSize"], pointer + "/byteSize");
            if (!byteSize) { auto error = byteSize.error(); error.path = manifestPath; return std::unexpected(std::move(error)); }
            entry.byteSize = *byteSize;
            if (entry.kind.empty() || !IsSafeRelative(entry.path) || !entry.path.starts_with("assets/"))
                return std::unexpected(MakeError(ArchiveErrorCode::UnsafePath, "asset path or kind is invalid", manifestPath, pointer));
            if (catalog->m_ByPath.contains(entry.path))
                return std::unexpected(MakeError(ArchiveErrorCode::InvalidFormat, "duplicate asset path", manifestPath, pointer + "/path"));
            auto safeFile = ResolveSafeFile(packageRoot, entry.path);
            if (!safeFile) { auto error = safeFile.error(); error.jsonPointer = pointer + "/path"; return std::unexpected(std::move(error)); }
            const auto actualSize = std::filesystem::file_size(*safeFile, ec);
            if (ec || actualSize != entry.byteSize)
                return std::unexpected(MakeError(ArchiveErrorCode::InvalidFormat, "asset byteSize does not match file", *safeFile, pointer + "/byteSize"));
            auto checked = CheckFileSize(*safeFile, actualSize, options.limits, totalBytes);
            if (!checked) return std::unexpected(checked.error());
            catalog->m_ByPath.emplace(entry.path, catalog->m_Entries.size());
            catalog->m_Entries.push_back(std::move(entry));
        }
        std::sort(catalog->m_Entries.begin(), catalog->m_Entries.end(), [](const auto& left, const auto& right) { return left.path < right.path; });
        catalog->m_ByPath.clear();
        for (std::size_t index = 0; index < catalog->m_Entries.size(); ++index)
            catalog->m_ByPath.emplace(catalog->m_Entries[index].path, index);

        LoadContext context;
        context.m_PackageRoot = packageRoot;
        context.m_SourceRoot = packageRoot;
        context.m_Assets = catalog;
        context.m_Limits = options.limits;
        auto worldDocument = Json::object();
        worldDocument["entities"] = manifest["entities"];
        auto world = World::Deserialize(worldDocument, codecs, context);
        if (!world)
        {
            auto error = world.error();
            if (error.path.empty()) error.path = manifestPath;
            return std::unexpected(std::move(error));
        }
        return LoadedWorldDirectory{std::move(*world), manifest["application"], packageRoot, std::move(catalog)};
    }
    catch (const std::bad_alloc&)
    {
        return std::unexpected(MakeError(ArchiveErrorCode::AllocationFailure, "allocation failed while loading world directory", source));
    }
    catch (const std::exception& exception)
    {
        return std::unexpected(MakeError(ArchiveErrorCode::InvalidFormat, exception.what(), source));
    }
}
} // namespace Aether::Serialization
