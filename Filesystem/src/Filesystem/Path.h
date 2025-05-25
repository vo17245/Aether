#pragma once
#include "Def.h"
#include <span>
#include "Core/String.h"
namespace Aether {
namespace Filesystem {

class Path
{
public:
    Path(const U8String& path) :
        m_Path(path)
    {
    }
    Path(const std::string_view path) :
        m_Path(path)
    {
    }
    Path(const char* path) :
        m_Path(path)
    {
    }
    Path(const std::string& path) :
        m_Path(path)
    {
    }
    Path() = default;
    Path(const Path& other) :
        m_Path(other.m_Path)
    {
    }
    Path(Path&& other) noexcept :
        m_Path(std::move(other.m_Path))
    {
    }
    Path operator/(const Path& other) const
    {
        return Path(m_Path + U8String("/") + other.m_Path);
    }
    Path& operator/=(const  Path& other)
    {
        m_Path += U8String("/") + other.m_Path;
        return *this;
    }
    Path operator=(const Path& other)
    {
        m_Path = other.m_Path;
        return *this;
    }
    operator std::string_view()const
    {
        auto view=m_Path.GetData();
        return std::string_view((char*)view.data(),view.size());
    }
    

public:
    U8String& GetStr()
    {
        return m_Path;
    }
    const U8String& GetStr() const
    {
        return m_Path;
    }
    Path Parent() const
    {
        auto pos = m_Path.FindLast(U8String("/"));
        if (!pos)
        {
            return Path();
        }
        return Path(m_Path.Sub(0, *pos));
    }
    bool Empty() const
    {
        return m_Path.CharCount() == 0;
    }

private:
    U8String m_Path; // Path is stored as UTF-8
};
}
} // namespace Aether::Filesystem