#pragma once
#include <vector>
#include <expected>
#include "IO/Json.h"
#include <string>
namespace Aether::Resource
{
template<typename T>
struct FromJsonImpl
{
    std::expected<T,std::string> operator()(const Json& json)
    {
        return json.get<T>();
    }
};
template<typename T>
inline std::expected<T,std::string> FromJson(const Json& json)
{
    return FromJsonImpl<T>{}(json);

}
template<typename T>
struct FromJsonImpl<std::vector<T>>
{
    std::expected<std::vector<T>,std::string> operator()(const Json& json)
    {
        std::vector<T> res;
        for (const auto& e : json)
        {
            res.push_back(FromJson<T>(e));
        }
        return res;
    }
};
}