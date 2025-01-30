#pragma once
#include "Def.h"
#include "Path.h"
#ifdef CreateDirectory
#undef CreateDirectory
#endif
namespace Aether::Filesystem
{
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

bool Exists(const std::string_view path);
bool IsDirectory(const std::string_view path);
size_t GetFileSize(FileHandle& handle);
bool CreateDirectory(const std::string_view path);
int64_t GetLastError();
bool RemoveFile(const std::string_view path);
bool RemoveDirectory(const std::string_view path);
std::optional<FindResult> FindFirst(const std::string_view path);
std::optional<FindResult> FindNext(FileHandle& handle);
} // namespace Aether::Filesystem