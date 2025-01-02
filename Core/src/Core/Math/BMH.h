#pragma once
#include <vector>
#include <span>
#include <unordered_map>
namespace Aether {
namespace Math {
// Boyer-Moore-Horspool 子串查找算法
template <typename T>
inline std::vector<size_t> BoyerMooreHorspool(const std::span<T> haystack, const std::span<T> needle)
{
    std::vector<size_t> result;

    if (needle.empty() || haystack.size() < needle.size())
    {
        return result;
    }

    // 计算偏移表（Bad Character Table）
    std::unordered_map<uint32_t, size_t> badCharTable;
    size_t needleLen = needle.size();

    for (size_t i = 0; i < needleLen - 1; ++i)
    {
        badCharTable[needle[i]] = needleLen - 1 - i;
    }

    size_t haystackLen = haystack.size();
    size_t offset = 0;

    while (offset <= haystackLen - needleLen)
    {
        size_t i = needleLen - 1;

        // 从后往前比较
        while (i < needleLen && haystack[offset + i] == needle[i])
        {
            --i;
        }

        // 如果完全匹配，将起始位置加入结果
        if (i == SIZE_MAX)
        {
            result.push_back(offset);
            offset += 1; // 匹配后移动一步以继续查找
        }
        else
        {
            // 根据偏移表移动
            size_t shift = badCharTable.count(haystack[offset + needleLen - 1]) ? badCharTable[haystack[offset + needleLen - 1]] : needleLen;
            offset += shift;
        }
    }

    return result;
}
}
} // namespace Aether::Math