#include <ProjectAsset/ProjectLock.h>

#include <system_error>

#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#else
#include <cerrno>
#include <fcntl.h>
#include <sys/file.h>
#include <unistd.h>
#endif

namespace Aether::ProjectAssets
{
struct ProjectWriteLock::State
{
    std::filesystem::path path;
#ifdef _WIN32
    HANDLE handle = INVALID_HANDLE_VALUE;
    OVERLAPPED overlapped{};
#else
    int fd = -1;
#endif
    ~State()
    {
#ifdef _WIN32
        if (handle != INVALID_HANDLE_VALUE)
        {
            UnlockFileEx(handle, 0, MAXDWORD, MAXDWORD, &overlapped);
            CloseHandle(handle);
        }
#else
        if (fd >= 0)
        {
            flock(fd, LOCK_UN);
            close(fd);
        }
#endif
    }
};

ProjectWriteLock::ProjectWriteLock(std::unique_ptr<State> state) : m_State(std::move(state)) {}
ProjectWriteLock::ProjectWriteLock(ProjectWriteLock&&) noexcept = default;
ProjectWriteLock& ProjectWriteLock::operator=(ProjectWriteLock&&) noexcept = default;

ProjectWriteLock::~ProjectWriteLock() = default;

Result<std::unique_ptr<ProjectWriteLock>> ProjectWriteLock::Acquire(const std::filesystem::path& projectRoot)
{
    if (projectRoot.empty()) return std::unexpected(Error{ErrorCode::InvalidArgument, "project root cannot be empty"});
    auto state = std::make_unique<State>();
    state->path = projectRoot / ".aether" / "catalog.lock";
    std::error_code ec;
    std::filesystem::create_directories(state->path.parent_path(), ec);
    if (ec) return std::unexpected(Error{ErrorCode::IoError, "could not create project lock directory: " + ec.message()});
#ifdef _WIN32
    state->handle = CreateFileW(state->path.c_str(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE,
                                nullptr, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (state->handle == INVALID_HANDLE_VALUE)
        return std::unexpected(Error{ErrorCode::IoError, "could not open project catalog lock"});
    if (!LockFileEx(state->handle, LOCKFILE_EXCLUSIVE_LOCK | LOCKFILE_FAIL_IMMEDIATELY, 0, MAXDWORD, MAXDWORD, &state->overlapped))
    {
        CloseHandle(state->handle);
        state->handle = INVALID_HANDLE_VALUE;
        return std::unexpected(Error{ErrorCode::Conflict, "project catalog is already open for writing"});
    }
#else
    state->fd = ::open(state->path.c_str(), O_CREAT | O_RDWR, 0600);
    if (state->fd < 0)
        return std::unexpected(Error{ErrorCode::IoError, "could not open project catalog lock: " + std::error_code(errno, std::system_category()).message()});
    if (flock(state->fd, LOCK_EX | LOCK_NB) != 0)
    {
        const auto conflict = errno == EWOULDBLOCK || errno == EAGAIN;
        close(state->fd);
        state->fd = -1;
        return std::unexpected(Error{conflict ? ErrorCode::Conflict : ErrorCode::IoError,
            conflict ? "project catalog is already open for writing" : "could not acquire project catalog lock: " + std::error_code(errno, std::system_category()).message()});
    }
#endif
    return std::unique_ptr<ProjectWriteLock>(new ProjectWriteLock(std::move(state)));
}

const std::filesystem::path& ProjectWriteLock::LockPath() const noexcept
{
    static const std::filesystem::path empty;
    return m_State ? m_State->path : empty;
}
}
