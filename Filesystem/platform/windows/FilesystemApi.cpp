#include "Filesystem/FilesystemApi.h"
#include "windows.h"
#ifdef CreateDirectory
#undef CreateDirectory
#endif
#include "Core/String.h"
#ifdef RemoveDirectory
#undef RemoveDirectory
#endif
namespace Aether {
static bool Utf8ToUtf16le(const Aether::U8String utf8str, std::wstring& utf16str)
{
    // 计算所需的 UTF-16 缓冲区大小
    int utf16len = MultiByteToWideChar(CP_UTF8,
                                       0, (char*)utf8str.GetData().data(),
                                       utf8str.GetData().size(),
                                       NULL, 0);
    // 分配缓冲区
    utf16str.resize(utf16len);
    // 进行转换
    int res = MultiByteToWideChar(CP_UTF8, 0,
                                  (char*)utf8str.GetData().data(),
                                  utf8str.GetData().size(),
                                  &utf16str[0], utf16len);
    return res != 0;
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
        //DWORD err = ::GetLastError();
        return false;
    }
    return true;
}
size_t Read(FileHandle& handle, std::span<uint8_t> buffer)
{
    DWORD bytesRead = 0;
    if (!::ReadFile(handle.data, buffer.data(), buffer.size_bytes(), &bytesRead, NULL))
    {
        //DWORD err = ::GetLastError();
    }
    return bytesRead;
}
size_t Write(FileHandle& handle, std::span<const uint8_t> buffer)
{
    DWORD bytesWritten = 0;
    if (!::WriteFile(handle.data, buffer.data(), buffer.size_bytes(), &bytesWritten, NULL))
    {
        //DWORD err = ::GetLastError();
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
        //DWORD err = ::GetLastError();
        return false;
    }
}
bool Exists(const Path& path)
{
    std::wstring wpath;
    if (!Utf8ToUtf16le(path.GetStr(), wpath))
    {
        return false;
    }
    DWORD attr = ::GetFileAttributesW(wpath.c_str());
    if (attr == INVALID_FILE_ATTRIBUTES)
    {
        //DWORD err = ::GetLastError();
        return false;
    }
    return true;
}
bool IsDirectory(const Path& path)
{
    std::wstring wpath;
    if (!Utf8ToUtf16le(path.GetStr(), wpath))
    {
        return false;
    }
    DWORD attr = ::GetFileAttributesW(wpath.c_str());
    if (attr == INVALID_FILE_ATTRIBUTES)
    {
        //DWORD err = ::GetLastError();
        return false;
    }
    return (attr & FILE_ATTRIBUTE_DIRECTORY) != 0;
}
size_t GetFileSize(FileHandle& handle)
{
    LARGE_INTEGER size;
    if (!::GetFileSizeEx(handle.data, &size))
    {
        //DWORD err = ::GetLastError();
        return 0;
    }
    return size.QuadPart;
}
bool CreateDirectory(const Path& path)
{
    std::wstring wpath;
    if (!Utf8ToUtf16le(path.GetStr(), wpath))
    {
        return false;
    }
    if (!::CreateDirectoryW(wpath.c_str(), NULL))
    {
        //DWORD err = ::GetLastError();
        return false;
    }
    return true;
}
bool RemoveFile(const Path& path)
{
    std::wstring wpath;
    if (!Utf8ToUtf16le(path.GetStr(), wpath))
    {
        return false;
    }
    if (!::DeleteFileW(wpath.c_str()))
    {
        //DWORD err = ::GetLastError();
        return false;
    }
    return true;
}
bool RemoveDirectory(const Path& path)
{
    std::wstring wpath;
    if (!Utf8ToUtf16le(path.GetStr(), wpath))
    {
        return false;
    }
    if (!::RemoveDirectoryW(wpath.c_str()))
    {
        //DWORD err = ::GetLastError();
        return false;
    }
    return true;
}
} // namespace Filesystem
} // namespace Aether::Filesystem