#pragma once
#include "IO/Json.h"
#include <vector>
namespace Aether::Resource
{
    template<typename T>
    struct ToJsonImpl
    {
        Json operator()(const T& t)
        {
            return t;
        }
    };
    template<typename T>
    inline Json ToJson(const T& t)
    {
        return ToJsonImpl<T>{}(t);
    }
    template<typename T>
    struct ToJsonImpl<std::vector<T>>
    {
        Json operator()(const std::vector<T>& t)
        {
            Json arr=Json::array();
            for (const auto& e : t)
            {
                arr.push_back(ToJson(e));
            }
            return arr;
        }
    };
}