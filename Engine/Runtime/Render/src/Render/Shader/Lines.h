#pragma once
#include <string>
namespace Aether {
namespace Shader {
class LineIterator
{
public:
    LineIterator(const std::string_view code, size_t offset, size_t length) :
        m_Code(code),m_Offset(offset),m_Length(length)
    {
    }

    LineIterator& operator++()
    {
        size_t i=m_Offset+m_Length+1;
        // skip this line crlf
        for(;i<m_Code.size();++i)
        {
            if(m_Code[i]!='\r'&&m_Code[i]!='\n')
            {
                break;
            }
        }
        // find next line
        m_Offset=i;
        for (; i < m_Code.size(); ++i)
        {
            if (m_Code[i] == '\n')
            {
                size_t len = i - m_Offset  ;
                if (i > 0 && m_Code[i - 1] == '\r')
                {
                    --len;
                }
                m_Length = len;
                return *this;
            }
        }
        // end of code
        m_Offset = m_Code.size();
        m_Length = 0;
        return *this;
    }
    bool operator!=(const LineIterator& other) const
    {
        return m_Offset!=other.m_Offset || m_Length!=other.m_Length||m_Code != other.m_Code;
    }
    const char* Data() const
    {
        return m_Code.data()+m_Offset;
    }
    size_t Size() const
    {
        return m_Length;
    }
    operator std::string_view() const
    {
        return std::string_view{m_Code.data() + m_Offset, m_Length};
    }
    std::string_view View() const
    {
        return std::string_view{m_Code.data() + m_Offset, m_Length};
    }
    bool operator==(const LineIterator& other) const
    {
        return !(*this != other);
    }
    bool operator==(const std::string_view& other) const
    {
        return View() == other;
    }
    bool operator!=(const std::string_view& other) const
    {
        return View() != other;
    }
    bool operator==(const char* other) const
    {
        return View() == other;
    }
    bool operator!=(const char* other) const
    {
        return View() != other;
    }
    std::string_view operator*() const
    {
        return View();
    }

private:
    const std::string_view m_Code;
    size_t m_Offset;
    size_t m_Length;
};
class Lines
{
public:
    Lines(const std::string_view code) :
        m_Code(code)
    {
    }
    LineIterator begin() const
    {
        size_t pos = 0;
        for (size_t i = 0; i < m_Code.size(); ++i)
        {
            if (m_Code[i] == '\n')
            {
                size_t len = i - pos;
                if (len > 0 && m_Code[pos + len - 1] == '\r')
                {
                    --len;
                }
                return LineIterator{m_Code, pos, len};
            }
        }
        return LineIterator{m_Code, 0, m_Code.size()};
    }
    LineIterator end() const
    {
        return LineIterator{m_Code, m_Code.size(), 0};
    }

private:
    const std::string_view m_Code;
};
}
} // namespace Aether::Shader