#pragma once
#include "Render/RenderApi/DeviceTexture.h"
#include <Render/RenderApi.h>
#include <expected>
#include <nlohmann/json.hpp>
#include <Core/Serialization.h>
#include <format>
#include <Resource/Resource.h>
#include <Core/Core.h>
namespace Aether::Sprite
{

struct AABB
{
    Vec2i min;
    Vec2i max;
};

struct AtlasInfo
{
    Resource::ColorSpace colorSpace;
    std::string imagePath;
    /**
     * frameRange in this basis:
     * 0------> image width
     * |
     * |
     * |
     * V
     * image height
     */
    std::vector<AABB> frameRange;
    static std::optional<AtlasInfo> Load(const std::string_view path);
    bool Save(const std::string_view path);
};
namespace Aether
{

} // namespace Aether
struct Atlas
{
    AtlasInfo info;
    Borrow<DeviceTexture> texture;
};
} // namespace Aether::Sprite
namespace Aether
{
template <>
struct ToJson<Sprite::AABB>
{
    Json operator()(const Sprite::AABB& t)
    {
        Json json;
        json["min"] = ToJson<Vec2i>()(t.min);
        json["max"] = ToJson<Vec2i>()(t.max);
        return json;
    }
};
template <>
struct FromJson<Sprite::AABB>
{
    std::expected<Sprite::AABB, std::string> operator()(const Json& json)
    {
        try
        {
            auto json_min = json["min"];
            auto minEx = FromJson<Vec2i>()(json_min);
            if (!minEx.has_value())
            {
                return std::unexpected<std::string>(std::format("failed to load min: {}", minEx.error()));
            }
            auto json_max = json["max"];
            auto maxEx = FromJson<Vec2i>()(json_max);
            if (!maxEx.has_value())
            {
                return std::unexpected<std::string>(std::format("failed to load max: {}", maxEx.error()));
            }
            Sprite::AABB aabb;
            aabb.min = minEx.value();
            aabb.max = maxEx.value();
            return aabb;
        }
        catch (...)
        {
            return std::unexpected<std::string>("unknown error");
        }
    }
};
template <>
struct ToJson<Sprite::AtlasInfo>
{
    Json operator()(const Sprite::AtlasInfo& t)
    {
        Json json;
        json["colorSpace"] = ToJson<Resource::ColorSpace>()(t.colorSpace);
        json["imagePath"] = t.imagePath;
        json["frameRange"] = ToJson<std::vector<Sprite::AABB>>{}(t.frameRange);
        return json;
    }
};
template <>
struct FromJson<Sprite::AtlasInfo>
{
    std::expected<Sprite::AtlasInfo, std::string> operator()(const Json& json)
    {
        try
        {
            Sprite::AtlasInfo info;
            if (!json.is_object())
            {
                return std::unexpected<std::string>("json is not an object");
            }
            auto& json_colorSpace = json["colorSpace"];
            auto colorSpaceEx = FromJson<Resource::ColorSpace>{}(json_colorSpace);
            if (!colorSpaceEx)
            {
                return std::unexpected<std::string>(std::format("failed to load colorSpace: {}", colorSpaceEx.error()));
            }
            info.colorSpace = colorSpaceEx.value();
            auto& json_imagePath = json["imagePath"];
            auto imagePathEx = FromJson<std::string>{}(json_imagePath);
            if (!imagePathEx)
            {
                return std::unexpected<std::string>(std::format("failed to load imagePath: {}", imagePathEx.error()));
            }
            info.imagePath = std::move(imagePathEx.value());
            auto& json_frameRange = json["frameRange"];
            auto frameRangeEx = FromJson<std::vector<Sprite::AABB>>{}(json_frameRange);
            if (!frameRangeEx)
            {
                return std::unexpected<std::string>(std::format("failed to load frameRange: {}", frameRangeEx.error()));
            }
            info.frameRange = std::move(frameRangeEx.value());

            return info;
        }
        catch (...)
        {
            return std::unexpected<std::string>("unknown error");
        }
    }
};
} // namespace Aether
