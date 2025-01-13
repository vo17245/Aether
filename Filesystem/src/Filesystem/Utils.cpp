#include "Utils.h"
#include "Def.h"
#include <cstdint>
#include <span>
namespace Aether {
namespace Filesystem {
std::optional<std::vector<char>> ReadFile(const Filesystem::Path& path)
{
    File file(path, Filesystem::Action::Read);
    if (!file.IsOpened())
    {
        return std::nullopt;
    }
    size_t fileSize = file.GetSize();
    std::vector<char> buffer(fileSize);
    size_t readBytes = file.Read({reinterpret_cast<uint8_t*>(buffer.data()), buffer.size()});
    if (readBytes != fileSize)
    {
        return std::nullopt;
    }
    return buffer;
}
bool WriteFile(const Filesystem::Path& path, const std::span<uint8_t> data)
{
    File file(path, Action::Create);
    if (!file.IsOpened())
    {
        return false;
    }
    size_t writtenBytes = file.Write({data.data(), data.size()});
    if (writtenBytes != data.size())
    {
        return false;
    }
    return true;
}
bool WriteFile(const Filesystem::Path& path, const std::span<char> data)
{
    return WriteFile(path, std::span<uint8_t>((uint8_t*)data.data(), data.size()));
}
bool RemoveTree(const std::string_view path)
{
    if(!Exists(path))
    {
        return true;
    }
    if(!IsDirectory(path))
    {
        return RemoveFile(path);
    }

}
bool CreateDirectories(const Path& path)
{
    if(Exists(path))
    {
        return true;
    }
    Path parent = path.Parent();
    if(!parent.Empty()&&!Exists(parent))
    {
        if(!CreateDirectories(parent))
        {
            return false;
        }
    }
    return CreateDirectory(path);
}
}
} // namespace Aether::Filesystem