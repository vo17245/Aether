#include "Filesystem/FilesystemApi.h"

#include <cerrno>
#include <cstdio>
#include <dirent.h>
#include <fcntl.h>
#include <filesystem>
#include <fnmatch.h>
#include <memory>
#include <string>
#include <system_error>
#include <sys/stat.h>
#include <unistd.h>
#include <utility>

namespace Aether::Filesystem {
namespace {

struct SearchState
{
    DIR* directory;
    std::string pattern;

    ~SearchState()
    {
        ::closedir(directory);
    }
};

std::optional<FindData> NextMatch(SearchState& state)
{
    while (auto* entry = ::readdir(state.directory))
    {
        if (::fnmatch(state.pattern.c_str(), entry->d_name, 0) != 0)
        {
            continue;
        }

        struct stat info;
        const bool isDirectory = ::fstatat(::dirfd(state.directory), entry->d_name, &info, 0) == 0
                                 && S_ISDIR(info.st_mode);
        return FindData{U8String(entry->d_name), isDirectory ? FileType::Directory : FileType::Regular};
    }
    return std::nullopt;
}

bool GetPathInfo(std::string_view path, struct stat& info)
{
    return ::stat(std::string(path).c_str(), &info) == 0;
}

} // namespace

int64_t GetLastError()
{
    return errno;
}

bool OpenFile(const Path& path, ActionFlags actions, FileHandle& handle)
{
    handle.data = nullptr;
    const bool create = (actions & Action::Create) != 0;
    const bool append = (actions & Action::Append) != 0;
    const bool read = (actions & Action::Read) != 0;
    const bool overwrite = (actions & Action::Overwrite) != 0;
    const bool write = create || append || overwrite;
    if (!read && !write)
    {
        errno = EINVAL;
        return false;
    }

    int flags = read ? (write ? O_RDWR : O_RDONLY) : O_WRONLY;
    if (create)
    {
        flags |= O_CREAT | O_TRUNC;
    }
    else if (overwrite)
    {
        flags |= O_TRUNC;
    }
    if (append)
    {
        flags |= O_APPEND;
    }

    const std::string filename = path.GetStr().ToStdString();
    const int fd = ::open(filename.c_str(), flags, 0666);
    if (fd == -1)
    {
        return false;
    }

    const char* mode = read ? (write ? (append ? "a+b" : "r+b") : "rb")
                            : (append ? "ab" : "wb");
    FILE* file = ::fdopen(fd, mode);
    if (!file)
    {
        const int error = errno;
        ::close(fd);
        errno = error;
        return false;
    }
    handle.data = file;
    return true;
}

size_t Read(FileHandle& handle, std::span<uint8_t> buffer)
{
    if (!handle.data || buffer.empty())
    {
        return 0;
    }
    return ::fread(buffer.data(), 1, buffer.size(), static_cast<FILE*>(handle.data));
}

size_t Write(FileHandle& handle, std::span<const uint8_t> buffer)
{
    if (!handle.data || buffer.empty())
    {
        return 0;
    }
    return ::fwrite(buffer.data(), 1, buffer.size(), static_cast<FILE*>(handle.data));
}

bool CloseFile(FileHandle& handle)
{
    if (!handle.data)
    {
        errno = EBADF;
        return false;
    }
    FILE* file = static_cast<FILE*>(handle.data);
    handle.data = nullptr;
    return ::fclose(file) == 0;
}

bool Exists(std::string_view path)
{
    struct stat info;
    return GetPathInfo(path, info);
}

bool IsDirectory(std::string_view path)
{
    struct stat info;
    return GetPathInfo(path, info) && S_ISDIR(info.st_mode);
}

bool IsFile(std::string_view path)
{
    struct stat info;
    return GetPathInfo(path, info) && S_ISREG(info.st_mode);
}

size_t GetFileSize(FileHandle& handle)
{
    if (!handle.data)
    {
        errno = EBADF;
        return 0;
    }
    FILE* file = static_cast<FILE*>(handle.data);
    if (::fflush(file) != 0)
    {
        return 0;
    }
    struct stat info;
    if (::fstat(::fileno(file), &info) != 0)
    {
        return 0;
    }
    return static_cast<size_t>(info.st_size);
}

bool CreateDirectory(std::string_view path)
{
    return ::mkdir(std::string(path).c_str(), 0777) == 0;
}

bool RemoveFile(std::string_view path)
{
    return ::unlink(std::string(path).c_str()) == 0;
}

bool RemoveDirectory(std::string_view path)
{
    return ::rmdir(std::string(path).c_str()) == 0;
}

std::optional<FindResult> FindFirst(std::string_view path)
{
    const size_t slash = path.find_last_of('/');
    const std::string directory = slash == std::string_view::npos ? "."
                                 : slash == 0 ? "/" : std::string(path.substr(0, slash));
    const std::string pattern(path.substr(slash == std::string_view::npos ? 0 : slash + 1));
    if (pattern.empty())
    {
        errno = EINVAL;
        return std::nullopt;
    }

    DIR* dir = ::opendir(directory.c_str());
    if (!dir)
    {
        return std::nullopt;
    }
    auto state = std::make_unique<SearchState>(dir, pattern);
    auto data = NextMatch(*state);
    if (!data)
    {
        return std::nullopt;
    }
    return FindResult{FileHandle{state.release()}, std::move(*data)};
}

std::optional<FindResult> FindNext(FileHandle& handle)
{
    if (!handle.data)
    {
        errno = EBADF;
        return std::nullopt;
    }
    auto* state = static_cast<SearchState*>(handle.data);
    auto data = NextMatch(*state);
    if (!data)
    {
        delete state;
        handle.data = nullptr;
        return std::nullopt;
    }
    return FindResult{handle, std::move(*data)};
}

std::optional<std::string> CopyFileFast(std::string_view srcPath, std::string_view destPath)
{
    std::error_code error;
    std::filesystem::copy_file(std::filesystem::path(std::string(srcPath)),
                               std::filesystem::path(std::string(destPath)),
                               std::filesystem::copy_options::overwrite_existing, error);
    if (error)
    {
        return "failed to copy file from " + std::string(srcPath) + " to "
               + std::string(destPath) + ": " + error.message();
    }
    return std::nullopt;
}

} // namespace Aether::Filesystem
