#include "Filesystem/FilesystemApi.h"
#include "Filesystem/Def.h"
#include "windows.h"
#ifdef CreateDirectory
#undef CreateDirectory
#endif
#include "Core/String.h"
#ifdef RemoveDirectory
#undef RemoveDirectory
#endif
namespace Aether {
static bool Utf8ToUtf16le(const std::string_view utf8str, std::wstring& utf16str)
{
    // 计算所需的 UTF-16 缓冲区大小
    int utf16len = MultiByteToWideChar(CP_UTF8,
                                       0, (char*)utf8str.data(),
                                       utf8str.size(),
                                       NULL, 0);
    // 分配缓冲区
    utf16str.resize(utf16len);
    // 进行转换
    int res = MultiByteToWideChar(CP_UTF8, 0,
                                  (char*)utf8str.data(),
                                  utf8str.size(),
                                  &utf16str[0], utf16len);
    return res != 0;
}
static bool Utf8ToUtf16le(const Aether::U8String& utf8str, std::wstring& utf16str)
{
    return Utf8ToUtf16le(std::string_view(utf8str), utf16str);
}
} // namespace Aether
namespace Aether {
namespace Filesystem {
int64_t GetLastError()
{
    return ::GetLastError();
}
bool OpenFile(const Path& path, ActionFlags actions, FileHandle& handle)
{
    std::wstring wpath;
    if (!Utf8ToUtf16le(path.GetStr(), wpath))
    {
        return false;
    }
    DWORD access = 0;
    DWORD share = 0;
    DWORD disposition = 0;
    DWORD flags = 0;
    if (actions & Action::Create)
    {
        access |= GENERIC_WRITE;
        disposition = CREATE_ALWAYS;
    }

    if (actions & Action::Append)
    {
        access |= FILE_APPEND_DATA;
    }
    if (actions & Action::Read)
    {
        access |= GENERIC_READ;
        disposition = OPEN_EXISTING;
    }
    if (actions & Action::Overwrite)
    {
        access |= GENERIC_WRITE;
        disposition = TRUNCATE_EXISTING;
    }

    handle.data = ::CreateFileW(wpath.c_str(), access, share, NULL, disposition, flags, NULL);
    if (handle.data == INVALID_HANDLE_VALUE)
    {
        // DWORD err = ::GetLastError();
        return false;
    }
    return true;
}
size_t Read(FileHandle& handle, std::span<uint8_t> buffer)
{
    DWORD bytesRead = 0;
    if (!::ReadFile(handle.data, buffer.data(), buffer.size_bytes(), &bytesRead, NULL))
    {
        // DWORD err = ::GetLastError();
    }
    return bytesRead;
}
size_t Write(FileHandle& handle, std::span<const uint8_t> buffer)
{
    DWORD bytesWritten = 0;
    if (!::WriteFile(handle.data, buffer.data(), buffer.size_bytes(), &bytesWritten, NULL))
    {
        // DWORD err = ::GetLastError();
    }
    return bytesWritten;
}
bool CloseFile(FileHandle& handle)
{
    if (::CloseHandle(handle.data))
    {
        handle.data = INVALID_HANDLE_VALUE;
        return true;
    }
    else
    {
        // DWORD err = ::GetLastError();
        return false;
    }
}
bool Exists(const std::string_view path)
{
    std::wstring wpath;
    if (!Utf8ToUtf16le(path, wpath))
    {
        return false;
    }
    DWORD attr = ::GetFileAttributesW(wpath.c_str());
    if (attr == INVALID_FILE_ATTRIBUTES)
    {
        // DWORD err = ::GetLastError();
        return false;
    }
    return true;
}
bool IsDirectory(const std::string_view path)
{
    std::wstring wpath;
    if (!Utf8ToUtf16le(path, wpath))
    {
        return false;
    }
    DWORD attr = ::GetFileAttributesW(wpath.c_str());
    if (attr == INVALID_FILE_ATTRIBUTES)
    {
        // DWORD err = ::GetLastError();
        return false;
    }
    return (attr & FILE_ATTRIBUTE_DIRECTORY) != 0;
}
size_t GetFileSize(FileHandle& handle)
{
    LARGE_INTEGER size;
    if (!::GetFileSizeEx(handle.data, &size))
    {
        // DWORD err = ::GetLastError();
        return 0;
    }
    return size.QuadPart;
}
bool CreateDirectory(const std::string_view path)
{
    std::wstring wpath;
    if (!Utf8ToUtf16le(path, wpath))
    {
        return false;
    }
    if (!::CreateDirectoryW(wpath.c_str(), NULL))
    {
        // DWORD err = ::GetLastError();
        return false;
    }
    return true;
}
bool RemoveFile(const std::string_view path)
{
    std::wstring wpath;
    if (!Utf8ToUtf16le(path, wpath))
    {
        return false;
    }
    if (!::DeleteFileW(wpath.c_str()))
    {
        // DWORD err = ::GetLastError();
        return false;
    }
    return true;
}
bool RemoveDirectory(const std::string_view path)
{
    std::wstring wpath;
    if (!Utf8ToUtf16le(path, wpath))
    {
        return false;
    }
    if (!::RemoveDirectoryW(wpath.c_str()))
    {
        // DWORD err = ::GetLastError();
        return false;
    }
    return true;
}
std::optional<FindResult> FindFirst(const std::string_view path)
{
    std::wstring wpath;
    if (!Utf8ToUtf16le(path, wpath))
    {
        return std::nullopt;
    }

    WIN32_FIND_DATAW findParam{};// 初始化 WIN32_FIND_DATAW
    HANDLE handle=FindFirstFileW(wpath.c_str(), &findParam);
    if(handle==INVALID_HANDLE_VALUE)
    {
        return std::nullopt;
    }
    FileHandle fileHandle;
    fileHandle.data=handle;
    FindData findData;
    findData.name=U8String(reinterpret_cast<const char*>(findParam.cFileName));
    if(findParam.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)
    {
        findData.type=FileType::Directory;
    }
    else
    {
        findData.type=FileType::Regular;
    }
    FindResult result;
    result.handle=fileHandle;
    result.findData=std::move(findData);
    return result;
}
std::optional<FindResult> FindNext(FileHandle& handle)
{
    WIN32_FIND_DATAW findParam{};
    if(!FindNextFileW(handle.data, &findParam))
    {
        return std::nullopt;
    }
    FindData findData;
    findData.name=U8String(reinterpret_cast<const char*>(findParam.cFileName));
    if(findParam.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)
    {
        findData.type=FileType::Directory;
    }
    else
    {
        findData.type=FileType::Regular;
    }
    FindResult result;
    result.handle=handle;
    result.findData=std::move(findData);
    return result;
}

}// namespace Filesystem
} // namespace Aether