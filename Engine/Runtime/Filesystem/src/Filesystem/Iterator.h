#pragma once
#include "FilesystemApi.h"
namespace Aether::Filesystem
{
class Entry
{
public:
    Entry(const Path& path, const FindResult& result) :
        m_Result(result)
    {
    }

    const U8String& GetName() const
    {
        return m_Result.findData.name;
    }
    bool IsDirectory() const
    {
        return m_Result.findData.type == FileType::Directory;
    }
    bool IsRegular() const
    {
        return m_Result.findData.type == FileType::Regular;
    }
    bool operator==(const Entry& other) const
    {
        return m_Result.findData.name == other.m_Result.findData.name&&
               m_Result.findData.type == other.m_Result.findData.type&&
               m_Result.handle.data == other.m_Result.handle.data;
    }
    Entry operator++()const
    {
        
    }


private:
    FindResult m_Result;
};
class Iterator
{
};
} // namespace Aether::Filesystem