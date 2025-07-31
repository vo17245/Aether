#pragma once
#include <cstddef>
#include <cstdint>
#include <shared_mutex>
#include <span>
#include <variant>
#include <vector>
#include <string>
#include <optional>
#include "Math.h"
#include "Math/BMH.h"
namespace Aether
{
using Unicode = uint32_t;
/**
 * @param out utf8 converted from unicode
 * @return the length of utf8
 */
uint8_t Unicode2Utf8(Unicode unicode, uint32_t& utf8);
class U32String
{
public: // constructors
    U32String(std::span<const uint32_t> data) :
        m_Data(data.begin(), data.end())
    {
    }
    U32String(const std::vector<uint32_t>& data) :
        m_Data(data)
    {
    }
    U32String(const U32String& other) = default;
    U32String(U32String&& other) = default;
    U32String() = default;
    U32String& operator=(const U32String& other) = default;
    U32String& operator=(U32String&& other) = default;
    U32String(const std::string_view data);
    void Push(Unicode unicode)
    {
        m_Data.push_back(unicode);
    }
    std::string ToStdString() const
    {
        std::string res;
        uint32_t utf8;
        uint8_t len;
        for (auto& unicode : m_Data)
        {
            len = Unicode2Utf8(unicode, utf8);
            res.append(reinterpret_cast<const char*>(&utf8), len);
        }
        return res;
    }

public: // operators
    U32String operator+(const U32String& other) const
    {
        U32String result(*this);
        result.m_Data.insert(result.m_Data.end(), other.m_Data.begin(), other.m_Data.end());
        return result;
    }
    U32String& operator+=(const U32String& other)
    {
        m_Data.insert(m_Data.end(), other.m_Data.begin(), other.m_Data.end());
        return *this;
    }
    bool operator==(const U32String& other) const
    {
        return m_Data == other.m_Data;
    }
    bool operator!=(const U32String& other) const
    {
        return m_Data != other.m_Data;
    }
    uint32_t& operator[](int i)
    {
        return m_Data[i];
    }
    const uint32_t& operator[](int i) const
    {
        return m_Data[i];
    }
    operator std::basic_string_view<uint32_t>()
    {
        return std::basic_string_view<uint32_t>(m_Data.data(), m_Data.size());
    }

public: // getters/setters
    size_t Size() const
    {
        return m_Data.size();
    }
    std::span<const uint32_t> GetDataView() const
    {
        return std::span<const uint32_t>(m_Data.data(), m_Data.size());
    }
    std::vector<uint32_t>& GetData()
    {
        return m_Data;
    }
    inline const std::vector<uint32_t>& GetData() const
    {
        return m_Data;
    }
    uint32_t& operator[](size_t index)
    {
        return m_Data[index];
    }
    const uint32_t& operator[](size_t index) const
    {
        return m_Data[index];
    }

public: // string utils
    std::vector<size_t> Find(const U32String& sub, size_t begin, size_t length) const
    {
        auto res = Math::BoyerMooreHorspool(
            std::span<const uint32_t>(m_Data.data() + begin, m_Data.data() + begin + length),
            sub.GetDataView());
        for (auto& val : res)
        {
            val += begin;
        }
        return res;
    }
    std::vector<size_t> Find(const U32String& sub) const
    {
        return Find(sub, 0, m_Data.size());
    }
    std::vector<size_t> Find(const std::string_view sub) const
    {
        U32String u32sub(sub);
        return Find(u32sub);
    }
    std::vector<size_t> Find(const std::string_view sub, size_t begin, size_t length) const
    {
        U32String u32sub(sub);
        return Find(u32sub, begin, length);
    }
    std::vector<U32String> Split(const U32String& delimiter) const
    {
        std::vector<U32String> res;
        size_t begin = 0;
        size_t end = 0;
        while (true)
        {
            auto pos = Find(delimiter, begin, m_Data.size() - begin);
            if (pos.empty())
            {
                res.push_back(std::span<const uint32_t>(m_Data.data() + begin, m_Data.data() + m_Data.size()));
                break;
            }
            end = pos[0];
            res.push_back(std::span<const uint32_t>(m_Data.data() + begin, m_Data.data() + end));
            begin = end + delimiter.Size();
        }
        return res;
    }
    std::vector<U32String> Split(const std::string_view delimiter) const
    {
        U32String u32delim(delimiter);
        return Split(u32delim);
    }

    std::optional<size_t> FindLast(const U32String& sub) const
    {
        auto res = Find(sub);
        if (res.empty())
        {
            return std::nullopt;
        }
        return res.back();
    }
    std::optional<size_t> FindLast(const std::string_view sub) const
    {
        auto u32sub = U32String(sub);
        return FindLast(u32sub);
    }
    std::optional<size_t> FindFirst(const U32String& sub) const
    {
        auto res = Find(sub);
        if (res.empty())
        {
            return std::nullopt;
        }
        return res.front();
    }
    std::optional<size_t> FindFirst(const std::string_view sub) const
    {
        auto u32sub = U32String(sub);
        return FindFirst(u32sub);
    }
    bool operator==(const std::string_view other) const
    {
        U32String u32other(other);
        return m_Data == u32other.GetData();
    }

private:
    std::vector<uint32_t> m_Data;
};

class U8String
{
public:
    U8String operator+(const U8String& other) const;
    U8String& operator=(const U8String& other);
    U8String(const char* str);
    U8String(const char8_t* str);
    U8String(const std::string& str);
    U8String(std::span<const uint8_t> data);
    U8String(const U8String& other) = default;
    U8String(U8String&& other) = default;
    U8String() = default;
    U8String(const std::string_view data);

public:
    std::span<const uint8_t> GetData() const
    {
        return std::span<const uint8_t>(m_Data.data(), m_Data.size());
    }

    bool PushUnicode(Unicode unicode);
    /**
     * @brief 获取第index个unicode
     */
    bool At(size_t index, Unicode& unicode) const;
    /**
     * @brief 获取第index个unicode并获取这个unicode的偏移字节数
     */
    bool At(size_t index, Unicode& unicode, size_t& offset) const;
    bool Next(size_t& offset, Unicode& unicode) const;
    size_t CharCount() const
    {
        return m_CharCount;
    }
    operator std::string() const
    {
        return std::string(m_Data.begin(), m_Data.end());
    }
    U32String ToU32String() const
    {
        U32String res;
        size_t offset = 0;
        Unicode c;
        while (offset < m_Data.size())
        {
            if (!Next(offset, c)) break;
            res.GetData().push_back(c);
        }
        return res;
    }
    operator U32String() const
    {
        return ToU32String();
    }

    U8String& operator+=(const U8String& other)
    {
        m_Data.insert(m_Data.end(), other.m_Data.begin(), other.m_Data.end());
        m_CharCount += other.m_CharCount;
        return *this;
    }
    U8String operator+(const char* str) const
    {
        U8String result(*this);
        while (*str)
        {
            uint8_t byte = *str;
            result.m_Data.push_back(byte);
            ++str;
        }
        // 计算字符数量
        size_t offset = 0;
        Unicode c;
        while (offset < result.m_Data.size())
        {
            if (!result.Next(offset, c)) break;
            result.m_CharCount++;
        }
        return result;
    }
    std::vector<size_t> Find(const U8String& sub, size_t beginBytes, size_t lengthBytes) const
    {
        size_t unicodeOffset = 0;
        {
            U32String skip;
            size_t offset = 0;
            Unicode c;
            while (offset < beginBytes)
            {
                if (!Next(offset, c)) break;
                skip.GetData().push_back(c);
            }
            unicodeOffset = skip.GetData().size();
        }

        U32String u32str;
        size_t offset = beginBytes;
        Unicode c;
        while (offset < beginBytes + lengthBytes)
        {
            if (!Next(offset, c)) break;
            u32str.GetData().push_back(c);
        }
        U32String u32sub = sub;
        auto res = Math::BoyerMooreHorspool(std::span<const uint32_t>(u32str.GetData().data(), u32str.GetData().data() + u32str.GetData().size()),
                                            std::span<const uint32_t>(u32sub.GetData().data(), u32sub.GetData().data() + u32sub.GetData().size()));
        for (auto& val : res)
        {
            val += unicodeOffset;
        }
        return res;
    }
    std::vector<size_t> Find(const U8String& sub) const
    {
        return Find(sub, 0, m_Data.size());
    }
    std::vector<U8String> Split(const U8String& delimiter) const
    {
        std::vector<U8String> res;
        size_t begin = 0;
        size_t end = 0;
        while (true)
        {
            auto pos = Find(delimiter, begin, m_Data.size() - begin);
            if (pos.empty())
            {
                size_t beginByte;
                Unicode c;
                At(begin, c, beginByte);
                res.push_back(std::span<const uint8_t>(m_Data.data() + beginByte, m_Data.data() + m_Data.size()));
                break;
            }
            end = pos[0];
            size_t beginByte;
            Unicode c;
            At(begin, c, beginByte);
            size_t endByte;
            At(end, c, endByte);
            res.push_back(std::span<const uint8_t>(m_Data.data() + beginByte, m_Data.data() + endByte));
            begin = end + delimiter.m_CharCount;
        }
        return res;
    }
    bool operator==(const U8String& other) const
    {
        return m_Data == other.m_Data;
    }
    std::optional<size_t> FindLast(const U8String& sub) const
    {
        auto res = Find(sub);
        if (res.empty())
        {
            return std::nullopt;
        }
        return res.back();
    }
    std::optional<size_t> FindFirst(const U8String& sub) const
    {
        auto res = Find(sub);
        if (res.empty())
        {
            return std::nullopt;
        }
        return res.front();
    }
    U8String Sub(size_t begin, size_t end) const
    {
        size_t beginByte;
        Unicode c;
        At(begin, c, beginByte);
        size_t endByte;
        At(end, c, endByte);
        return std::span<const uint8_t>(m_Data.data() + beginByte, m_Data.data() + endByte);
    }
    operator std::string_view() const
    {
        return std::string_view((char*)m_Data.data(), m_Data.size());
    }
    std::string ToStdString() const
    {
        return std::string((char*)m_Data.data(), m_Data.size());
    }

private:
    std::vector<uint8_t> m_Data;
    size_t m_CharCount = 0;
};
inline bool Utf8ToUtf32(/*in*/ const std::span<uint8_t> u8, /*inout*/ size_t& offset, /*out*/ uint32_t& u32)
{
    u32 = 0;
    uint8_t first_byte = u8[offset];
    size_t length = 0;

    // 确定 UTF-8 序列的字节数
    if (first_byte <= 0x7F)
    {
        length = 1;
        u32 = first_byte;
    }
    else if ((first_byte & 0xE0) == 0xC0)
    {
        length = 2;
        u32 = first_byte & 0x1F;
    }
    else if ((first_byte & 0xF0) == 0xE0)
    {
        length = 3;
        u32 = first_byte & 0x0F;
    }
    else if ((first_byte & 0xF8) == 0xF0)
    {
        length = 4;
        u32 = first_byte & 0x07;
    }
    else
    {
        // Invalid UTF-8 sequence
        return false;
    }

    // 验证剩余字节数是否足够
    if (offset + length > u8.size())
    {
        //("Incomplete UTF-8 sequence")
        return false;
    }

    // 解码剩余字节
    for (size_t i = 1; i < length; ++i)
    {
        uint8_t byte = u8[offset + i];
        if ((byte & 0xC0) != 0x80)
        {
            //("Invalid UTF-8 continuation byte");
            return false;
        }
        u32 = (u32 << 6) | (byte & 0x3F);
    }

    // 更新索引
    offset += length;
    return true;
}

} // namespace Aether
