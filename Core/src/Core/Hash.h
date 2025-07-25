#pragma once
#include <unordered_map>// For std::hash
namespace Aether
{
    template<typename T>
    struct Hash
    {
        std::size_t operator()(const T& value) const
        {
            return std::hash<T>()(value);
        }
    };
}