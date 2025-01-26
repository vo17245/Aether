#pragma once
#include "Def.h"
#include "Filesystem.h"
#include "Filesystem/Filesystem.h"
#include "FilesystemApi.h"
namespace Aether {
namespace Filesystem {

class File
{
public:
    File(const Path& path, ActionFlags actions) :
        m_Path(path)
    {
        Open(actions);
    }
    File(const Path& path, Action actions) :
        m_Path(path)
    {
        Open(static_cast<ActionFlags>(actions));
    }
    bool Open(ActionFlags actions)
    {
        bool res = OpenFile(m_Path, actions, m_Handle);
        m_Opened = res;
        return res;
    }
    bool Close()
    {
        bool res = CloseFile(m_Handle);
        m_Opened = false;
        return res;
    }
    size_t Read(std::span<uint8_t> buffer)
    {
        return ::Aether::Filesystem::Read(m_Handle, buffer);
    }
    size_t Write(const std::span<const uint8_t> buffer)
    {
        return ::Aether::Filesystem::Write(m_Handle, buffer);
    }
    bool IsOpened() const
    {
        return m_Opened;
    }
    const Path& GetPath() const
    {
        return m_Path;
    }
    FileHandle GetHandle() const
    {
        return m_Handle;
    }
    ~File()
    {
        if (m_Opened)
        {
            Close();
        }
    }
    size_t GetSize()
    {
        return GetFileSize(m_Handle);
    }
    static bool Exists(const Path& path)
    {
        return ::Aether::Filesystem::Exists(path) && !IsDirectory(path);
    }

private:
    Path m_Path;
    FileHandle m_Handle;
    bool m_Opened = false;
};
}
} // namespace Aether::Filesystem