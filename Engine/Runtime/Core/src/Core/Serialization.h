#pragma once
#include <nlohmann/json.hpp>
#include <expected>
#include <vector>
#include "Math.h"
#include "Math/Def.h"
namespace Aether
{
using Json = nlohmann::json;
template <typename T>
struct ToJson
{
    Json operator()(const T& t)
    {
        static_assert(sizeof(T) == 0, "No ToJson implementation for this type");
        return t.ToJson();
    }
};
template <typename T>
inline Json ToJsonI(const T& t)
{
    return ToJson<T>()(t);
}
template<typename T>
inline std::string ToJsonStr(const T& t)
{
    Json json = ToJsonI(t);
    return json.dump();
}
template <typename T>
struct FromJson
{
    std::expected<T, std::string> operator()(const Json& json)
    {
        static_assert(sizeof(T) == 0, "No FromJson implementation for this type");
        return T::FromJson(json);
    }
};
template <typename T>
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
template <typename T>
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
template <>
struct ToJson<Vec2i>
{
    Json operator()(const Vec2i& t)
    {
        return {{"x", t.x()}, {"y", t.y()}};
    }
};
template <>
struct FromJson<Vec2i>
{
    std::expected<Vec2i, std::string> operator()(const Json& json)
    {
        auto& json_x = json["x"];
        if (!json_x.is_number_integer())
        {
            return std::unexpected<std::string>("x is not an integer");
        }
        auto& json_y = json["y"];
        if (!json_y.is_number_integer())
        {
            return std::unexpected<std::string>("y is not an integer");
        }
        return Vec2i(json_x.get<int>(), json_y.get<int>());
    }
};
// implement for base type
template <>
struct ToJson<std::string>
{
    Json operator()(const std::string& t)
    {
        return t;
    }
};
template <>
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
template <>
struct ToJson<Vec4f>
{
    Json operator()(const Vec4f& t)
    {
        float x = t[0];
        float y = t[1];
        float z = t[2];
        float w = t[3];
        return {{"x", x}, {"y", y}, {"z", z}, {"w", w}};
    }
};
template <>
struct FromJson<Vec4f>
{
    std::expected<Vec4f, std::string> operator()(const Json& json)
    {
        auto& json_x = json["x"];
        if (!json_x.is_number_float() && !json_x.is_number_integer())
        {
            return std::unexpected<std::string>("x is not a number");
        }
        auto& json_y = json["y"];
        if (!json_y.is_number_float() && !json_y.is_number_integer())
        {
            return std::unexpected<std::string>("y is not a number");
        }
        auto& json_z = json["z"];
        if (!json_z.is_number_float() && !json_z.is_number_integer())
        {
            return std::unexpected<std::string>("z is not a number");
        }
        auto& json_w = json["w"];
        if (!json_w.is_number_float() && !json_w.is_number_integer())
        {
            return std::unexpected<std::string>("w is not a number");
        }
        return Vec4f(json_x.get<float>(), json_y.get<float>(), json_z.get<float>(), json_w.get<float>());
    }
};
template <>
struct ToJson<Vec3f>
{
    Json operator()(const Vec3f& t)
    {
        float x = t[0];
        float y = t[1];
        float z = t[2];
        return {{"x", x}, {"y", y}, {"z", z}};
    }
};
template <>
struct FromJson<Vec3f>
{
    std::expected<Vec3f, std::string> operator()(const Json& json)
    {
        auto& json_x = json["x"];
        if (!json_x.is_number_float() && !json_x.is_number_integer())
        {
            return std::unexpected<std::string>("x is not a number");
        }
        auto& json_y = json["y"];
        if (!json_y.is_number_float() && !json_y.is_number_integer())
        {
            return std::unexpected<std::string>("y is not a number");
        }
        auto& json_z = json["z"];
        if (!json_z.is_number_float() && !json_z.is_number_integer())
        {
            return std::unexpected<std::string>("z is not a number");
        }
        return Vec3f(json_x.get<float>(), json_y.get<float>(), json_z.get<float>());
    }
};
template <>
struct ToJson<Vec2f>
{
    Json operator()(const Vec2f& t)
    {
        float x = t[0];
        float y = t[1];
        return {{"x", x}, {"y", y}};
    }
};
template <>
struct FromJson<Vec2f>
{
    std::expected<Vec2f, std::string> operator()(const Json& json)
    {
        auto& json_x = json["x"];
        if (!json_x.is_number_float() && !json_x.is_number_integer())
        {
            return std::unexpected<std::string>("x is not a number");
        }
        auto& json_y = json["y"];
        if (!json_y.is_number_float() && !json_y.is_number_integer())
        {
            return std::unexpected<std::string>("y is not a number");
        }
        return Vec2f(json_x.get<float>(), json_y.get<float>());
    }
};
template <>
struct ToJson<Mat4f>
{
    Json operator()(const Mat4f& t)
    {
        Json jsonArray = Json::array();
        for (int i = 0; i < 16; ++i)
        {
            jsonArray.push_back(t.data()[i]);
        }
        return jsonArray;
    }
};
template <>
struct FromJson<Mat4f>
{
    std::expected<Mat4f, std::string> operator()(const Json& json)
    {
        if (!json.is_array() || json.size() != 16)
        {
            return std::unexpected<std::string>("Expected an array of size 16");
        }
        Mat4f mat;
        for (int i = 0; i < 16; ++i)
        {
            if (!json[i].is_number_float() && !json[i].is_number_integer())
            {
                return std::unexpected<std::string>("Element is not a number");
            }
            mat.data()[i] = json[i].get<float>();
        }
        return mat;
    }
};

} // namespace Aether