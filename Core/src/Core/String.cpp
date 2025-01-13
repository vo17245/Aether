#include "String.h"
#include <cstdint>
namespace Aether {
U8String::U8String(const std::string_view data)
{
    // 数据拷贝
    for (auto byte : data)
    {
        m_Data.push_back(byte);
    }
    // 计算字符数量
    size_t offset = 0;
    Unicode c;
    while (offset < m_Data.size())
    {
        if (!Next(offset, c)) break;
        m_CharCount++;
    }
}
uint8_t Unicode2Utf8(Unicode unicode, uint32_t& utf8)
{
    uint8_t* res = (uint8_t*)&utf8;
    if (unicode <= 0x7F)
    {
        // 单字节 (ASCII)
        res[0] = (static_cast<uint8_t>(unicode));
        return 1;
    }
    else if (unicode <= 0x7FF)
    {
        // 双字节
        res[0] = (static_cast<uint8_t>((unicode >> 6) | 0xC0));
        res[1] = (static_cast<uint8_t>((unicode & 0x3F) | 0x80));
        return 2;
    }
    else if (unicode <= 0xFFFF)
    {
        // 三字节
        res[0] = (static_cast<uint8_t>((unicode >> 12) | 0xE0));
        res[1] = (static_cast<uint8_t>(((unicode >> 6) & 0x3F) | 0x80));
        res[2] = (static_cast<uint8_t>((unicode & 0x3F) | 0x80));
        return 3;
    }
    else if (unicode <= 0x10FFFF)
    {
        // 四字节
        res[0] = (static_cast<uint8_t>((unicode >> 18) | 0xF0));
        res[1] = (static_cast<uint8_t>(((unicode >> 12) & 0x3F) | 0x80));
        res[2] = (static_cast<uint8_t>(((unicode >> 6) & 0x3F) | 0x80));
        res[3] = (static_cast<uint8_t>((unicode & 0x3F) | 0x80));
        return 4;
    }
    else
    {
        return 0; // error
    }
}
bool U8String::PushUnicode(Unicode unicode)
{
    uint32_t utf8;
    auto len = Unicode2Utf8(unicode, utf8);
    if (len == 0)
    {
        return false;
    }
    m_Data.insert(m_Data.end(), (uint8_t*)&utf8, (uint8_t*)&utf8 + len);
    ++m_CharCount;
    return true;
}
bool U8String::At(size_t index, Unicode& unicode) const
{
    size_t offset = 0;
    for (size_t i = 0; i < index; ++i)
    {
        if (!Next(offset, unicode))
        {
            return false;
        }
    }
    return true;
}
bool U8String::At(size_t index, Unicode& unicode, size_t& offset) const
{
    offset = 0;
    for (size_t i = 0; i < index; ++i)
    {
        if (!Next(offset, unicode))
        {
            return false;
        }
    }
    return true;
}
bool U8String::Next(size_t& offset, Unicode& unicode) const
{
    unicode = 0;
    uint8_t first_byte = m_Data[offset];
    size_t length = 0;

    // 确定 UTF-8 序列的字节数
    if (first_byte <= 0x7F)
    {
        length = 1;
        unicode = first_byte;
    }
    else if ((first_byte & 0xE0) == 0xC0)
    {
        length = 2;
        unicode = first_byte & 0x1F;
    }
    else if ((first_byte & 0xF0) == 0xE0)
    {
        length = 3;
        unicode = first_byte & 0x0F;
    }
    else if ((first_byte & 0xF8) == 0xF0)
    {
        length = 4;
        unicode = first_byte & 0x07;
    }
    else
    {
        // Invalid UTF-8 sequence
        return false;
    }

    // 验证剩余字节数是否足够
    if (offset + length > m_Data.size())
    {
        //("Incomplete UTF-8 sequence")
        return false;
    }

    // 解码剩余字节
    for (size_t i = 1; i < length; ++i)
    {
        uint8_t byte = m_Data[offset + i];
        if ((byte & 0xC0) != 0x80)
        {
            //("Invalid UTF-8 continuation byte");
            return false;
        }
        unicode = (unicode << 6) | (byte & 0x3F);
    }

    // 更新索引
    offset += length;
    return true;
}
U8String::U8String(const char* str)
{
    // 数据拷贝
    while (*str)
    {
        uint8_t byte = *str;
        m_Data.push_back(byte);
        ++str;
    }
    // 计算字符数量
    size_t offset = 0;
    Unicode c;
    while (offset < m_Data.size())
    {
        if (!Next(offset, c)) break;
        m_CharCount++;
    }
}
U8String::U8String(const std::string& str)
{
    // 数据拷贝
    for (auto byte : str)
    {
        m_Data.push_back(byte);
    }
    // 计算字符数量
    size_t offset = 0;
    Unicode c;
    while (offset < m_Data.size())
    {
        if (!Next(offset, c)) break;
        m_CharCount++;
    }
}
U8String::U8String(std::span<const uint8_t> data)
{
    // 数据拷贝
    for (auto byte : data)
    {
        m_Data.push_back(byte);
    }
    // 计算字符数量
    size_t offset = 0;
    Unicode c;
    while (offset < m_Data.size())
    {
        if (!Next(offset, c)) break;
        m_CharCount++;
    }
}
U8String::U8String(const char8_t* str)
{
    // 数据拷贝
    while (*str)
    {
        uint8_t byte = *str;
        m_Data.push_back(byte);
        ++str;
    }
    // 计算字符数量
    size_t offset = 0;
    Unicode c;
    while (offset < m_Data.size())
    {
        if (!Next(offset, c)) break;
        m_CharCount++;
    }
}
U8String U8String::operator+(const U8String& other) const
{
    U8String result(*this);
    result.m_Data.insert(result.m_Data.end(), other.m_Data.begin(), other.m_Data.end());
    result.m_CharCount += other.m_CharCount;
    return result;
}
U8String& U8String::operator=(const U8String& other)
{
    m_Data = other.m_Data;
    m_CharCount = other.m_CharCount;
    return *this;
}
U32String::U32String(const std::string_view data)
{
    U8String u8str(data);
    *this = u8str.ToU32String();
}
} // namespace Aether
