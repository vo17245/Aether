#pragma once
#include "Def.h"
#include "FilesystemApi.h"
#ifdef CreateDirectory
#undef CreateDirectory
#endif
namespace Aether
{
namespace Filesystem
{
class Directory
{
public:
    static bool Exists(const Path& path)
    {
        return Filesystem::Exists(path) && Filesystem::IsDirectory(path);
    }
    static bool CreateDirectories(const Path& path)
    {
        if (Exists(path))
        {
            return true;
        }
        Path parent = path.Parent();
        if (!parent.Empty() && !Exists(parent))
        {
            if (!CreateDirectories(parent))
            {
                return false;
            }
        }
        return Aether::Filesystem::CreateDirectory(path);
    }
};
} // namespace Filesystem
} // namespace Aether