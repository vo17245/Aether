#pragma once
#include <nlohmann/json.hpp>
#include <expected>
#include <vector>
#include "Math.h"
#include "Math/Def.h"
namespace Aether 
{
    using Json= nlohmann::json;
    template<typename T>
    struct ToJson 
    {
        Json operator()(const T& t) 
        {
            static_assert(sizeof(T)==0,"No ToJson implementation for this type");
            return t.ToJson();
        }
    };
    template<typename T>
    struct FromJson 
    {
        std::expected<T, std::string> operator()(const Json& json) 
        {
            static_assert(sizeof(T)==0,"No FromJson implementation for this type");
            return T::FromJson(json);
        }
    };
    template<typename T>
    struct ToJson<std::vector<T>>
    {
        Json operator()(const std::vector<T>& t)
        {
            Json jsonArray = Json::array();
            for (const auto& item : t) 
            {
                jsonArray.push_back(ToJson<T>()(item));
            }
            return jsonArray;
        }
    };
    template<typename T>
    struct FromJson<std::vector<T>>
    {
        std::expected<std::vector<T>, std::string> operator()(const Json& json)
        {
            if (!json.is_array()) 
            {
                return std::unexpected<std::string>("Expected an array");
            }
            std::vector<T> result;
            for (const auto& item : json) 
            {
                auto itemEx = FromJson<T>()(item);
                if (!itemEx) 
                {
                    return std::unexpected<std::string>(itemEx.error());
                }
                result.push_back(itemEx.value());
            }
            return result;
        }
    };
    // implement for math
    template<>
    struct ToJson<Vec2i>
    {
        Json operator()(const Vec2i& t) 
        {
            return {{"x", t.x()}, {"y", t.y()}};
        }
    };
    template<>
    struct FromJson<Vec2i>
    {
        std::expected<Vec2i,std::string> operator()(const Json& json)
        {
            auto& json_x=json["x"];
            if(!json_x.is_number_integer())
            {
                return std::unexpected<std::string>("x is not an integer");
            }
            auto& json_y=json["y"];
            if(!json_y.is_number_integer())
            {
                return std::unexpected<std::string>("y is not an integer");
            }
            return Vec2i(json_x.get<int>(), json_y.get<int>());
        }
    };
    //implement for base type
    template<>
    struct ToJson<std::string>
    {
        Json operator()(const std::string& t) 
        {
            return t;
        }  
    };
    template<>
    struct FromJson<std::string>
    {
        std::expected<std::string, std::string> operator()(const Json& json) 
        {
            if (!json.is_string()) 
            {
                return std::unexpected<std::string>("Expected a string");
            }
            return json.get<std::string>();
        }
    };

    

}