#pragma once
#include "Def.h"
#include "Path.h"
namespace Aether {
namespace Filesystem {
bool OpenFile(const Path& path, ActionFlags actions, FileHandle& handle);
inline bool OpenFile(const Path& path, Action actions, FileHandle& handle)
{
    return OpenFile(path, static_cast<ActionFlags>(actions), handle);
}
size_t Read(FileHandle& handle, std::span<uint8_t> buffer);
size_t Write(FileHandle& handle, std::span<const uint8_t> buffer);
inline size_t Write(FileHandle& handle, std::span<const char> buffer)
{
    return Write(handle, {reinterpret_cast<const uint8_t*>(buffer.data()), buffer.size()});
}
bool CloseFile(FileHandle& handle);
bool Exists(const Path& path);
bool IsDirectory(const Path& path);
size_t GetFileSize(FileHandle& handle);
bool CreateDirectory(const Path& path);
int64_t GetLastError();
bool RemoveFile(const Path& path);
bool RemoveDirectory(const Path& path);
}
} // namespace Aether::Filesystem