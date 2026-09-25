#include <EditorFramework/Storage/AtomicReplaceFile.h>

#include <Core/UUID.h>

#include <cerrno>
#include <algorithm>
#include <system_error>

#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#else
#include <fcntl.h>
#include <unistd.h>
#endif

namespace Aether::EditorFramework::Storage
{
namespace
{
std::string SystemError(std::string_view operation)
{
#ifdef _WIN32
    return std::string(operation) + " failed with Windows error " + std::to_string(GetLastError());
#else
    return std::string(operation) + ": " + std::error_code(errno, std::system_category()).message();
#endif
}

class NativeAtomicFileOperations final : public AtomicFileOperations
{
public:
    FileResult Write(const std::filesystem::path& temporary, std::span<const std::byte> contents) override
    {
#ifdef _WIN32
        HANDLE handle = CreateFileW(temporary.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_NEW,
                                    FILE_ATTRIBUTE_NORMAL, nullptr);
        if (handle == INVALID_HANDLE_VALUE) return std::unexpected(SystemError("create temporary file"));
        std::size_t offset = 0;
        while (offset < contents.size())
        {
            const auto amount = static_cast<DWORD>(std::min<std::size_t>(contents.size() - offset, 1u << 30));
            DWORD written = 0;
            if (!WriteFile(handle, contents.data() + offset, amount, &written, nullptr) || written == 0)
            {
                const auto error = SystemError("write temporary file");
                CloseHandle(handle);
                return std::unexpected(error);
            }
            offset += written;
        }
        if (!CloseHandle(handle)) return std::unexpected(SystemError("close temporary file"));
#else
        const int fd = ::open(temporary.c_str(), O_WRONLY | O_CREAT | O_EXCL, 0600);
        if (fd < 0) return std::unexpected(SystemError("create temporary file"));
        std::size_t offset = 0;
        while (offset < contents.size())
        {
            const auto written = ::write(fd, contents.data() + offset, contents.size() - offset);
            if (written < 0 && errno == EINTR) continue;
            if (written <= 0)
            {
                const auto error = SystemError("write temporary file");
                ::close(fd);
                return std::unexpected(error);
            }
            offset += static_cast<std::size_t>(written);
        }
        if (::close(fd) != 0) return std::unexpected(SystemError("close temporary file"));
#endif
        return {};
    }

    FileResult Flush(const std::filesystem::path& temporary) override
    {
#ifdef _WIN32
        HANDLE handle = CreateFileW(temporary.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING,
                                    FILE_ATTRIBUTE_NORMAL, nullptr);
        if (handle == INVALID_HANDLE_VALUE) return std::unexpected(SystemError("open temporary file for flush"));
        const bool result = FlushFileBuffers(handle) != 0;
        const auto error = result ? std::string{} : SystemError("flush temporary file");
        CloseHandle(handle);
        if (!result) return std::unexpected(error);
#else
        const int fd = ::open(temporary.c_str(), O_RDONLY);
        if (fd < 0) return std::unexpected(SystemError("open temporary file for flush"));
        const int result = ::fsync(fd);
        const auto error = result == 0 ? std::string{} : SystemError("flush temporary file");
        ::close(fd);
        if (result != 0) return std::unexpected(error);
#endif
        return {};
    }

    FileResult Replace(const std::filesystem::path& temporary, const std::filesystem::path& destination) override
    {
#ifdef _WIN32
        if (!MoveFileExW(temporary.c_str(), destination.c_str(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH))
            return std::unexpected(SystemError("replace destination file"));
#else
        if (::rename(temporary.c_str(), destination.c_str()) != 0)
            return std::unexpected(SystemError("replace destination file"));
#endif
        return {};
    }

    void Remove(const std::filesystem::path& path) noexcept override
    {
        std::error_code ignored;
        std::filesystem::remove(path, ignored);
    }
};
}

FileResult AtomicReplaceFile(const std::filesystem::path& destination, std::span<const std::byte> contents,
                             AtomicFileOperations* operations)
{
    if (destination.empty() || destination.filename().empty())
        return std::unexpected("destination path must name a file");
    std::error_code ec;
    if (!destination.parent_path().empty())
        std::filesystem::create_directories(destination.parent_path(), ec);
    if (ec) return std::unexpected("could not create destination directory: " + ec.message());

    NativeAtomicFileOperations native;
    auto& io = operations ? *operations : static_cast<AtomicFileOperations&>(native);
    const auto temporary = destination.parent_path() /
        (destination.filename().string() + ".aether-tmp-" + UUID::Create().ToString());
    auto written = io.Write(temporary, contents);
    if (!written)
    {
        io.Remove(temporary);
        return std::unexpected(written.error());
    }
    auto flushed = io.Flush(temporary);
    if (!flushed)
    {
        io.Remove(temporary);
        return std::unexpected(flushed.error());
    }
    auto replaced = io.Replace(temporary, destination);
    if (!replaced)
    {
        io.Remove(temporary);
        return std::unexpected(replaced.error());
    }
    return {};
}
}
