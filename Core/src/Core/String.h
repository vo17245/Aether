#pragma once
#include <cstdint>
#include <span>
#include <variant>
#include <vector>
#include <string>
#include <optional>
#include "Math.h"
#include "Math/BMH.h"
namespace Aether {

using Unicode = uint32_t;
using U32String = std::vector<Unicode>;
/**
 * @param out utf8 converted from unicode
 * @return the length of utf8
 */
uint8_t Unicode2Utf8(Unicode unicode, uint32_t& utf8);
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
    operator U32String() const
    {
        U32String res;
        size_t offset = 0;
        Unicode c;
        while (offset < m_Data.size())
        {
            if (!Next(offset, c)) break;
            res.push_back(c);
        }
        return res;
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
    std::vector<size_t> Find(const U8String& sub,size_t beginBytes,size_t lengthBytes)const
    {
        size_t unicodeOffset=0;
        {
            U32String skip;
            size_t offset = 0;
            Unicode c;
            while (offset < beginBytes)
            {
                if (!Next(offset, c)) break;
                skip.push_back(c);
            }
            unicodeOffset=skip.size();
        }

        U32String u32str;
        size_t offset = beginBytes;
        Unicode c;
        while (offset < beginBytes+lengthBytes)
        {
            if (!Next(offset, c)) break;
            u32str.push_back(c);
        }
        U32String u32sub=sub;
        auto res=Math::BoyerMooreHorspool(std::span<uint32_t>(u32str.data(),u32str.data()+u32str.size()),
         std::span<uint32_t>(u32sub.data(),u32sub.data()+u32sub.size()));
        for(auto& val:res)
        {
            val+=unicodeOffset;
        }
        return res;
    }
    std::vector<size_t> Find(const U8String& sub)const
    {
        return Find(sub,0,m_Data.size());
    }
    std::vector<U8String> Split(const U8String& delimiter)const
    {
        std::vector<U8String> res;
        size_t begin=0;
        size_t end=0;
        while(true)
        {
            auto pos=Find(delimiter,begin,m_Data.size()-begin);
            if(pos.empty())
            {
                size_t beginByte;
                Unicode c;
                At(begin,c,beginByte);
                res.push_back(std::span<const uint8_t>(m_Data.data()+beginByte,m_Data.data()+m_Data.size()));
                break;
            }
            end=pos[0];
            size_t beginByte;
            Unicode c;
            At(begin,c,beginByte);
            size_t endByte;
            At(end,c,endByte);
            res.push_back(std::span<const uint8_t>(m_Data.data()+beginByte,m_Data.data()+endByte));
            begin=end+delimiter.m_CharCount;
        }
        return res;
    }
    bool operator==(const U8String& other) const
    {
        return m_Data == other.m_Data;
    }
    std::optional<size_t> FindLast(const U8String& sub)const
    {
        auto res=Find(sub);
        if(res.empty())
        {
            return std::nullopt;
        }
        return res.back();
    }
    std::optional<size_t> FindFirst(const U8String& sub)const
    {
        auto res=Find(sub);
        if(res.empty())
        {
            return std::nullopt;
        }
        return res.front();
    }
    U8String Sub(size_t begin,size_t end)const
    {
        size_t beginByte;
        Unicode c;
        At(begin,c,beginByte);
        size_t endByte;
        At(end,c,endByte);
        return std::span<const uint8_t>(m_Data.data()+beginByte,m_Data.data()+endByte);
    }

private:
    std::vector<uint8_t> m_Data;
    size_t m_CharCount = 0;
};
} // namespace Aether
