#pragma once
#include "AssetInfo.h"
#include <Core/Serialization.h>
namespace Aether::Resource
{
enum class ColorSpace
{
    SRGB,
    LINEAR,
};
struct ImageInfo : public AssetInfo
{
    ImageInfo(const std::string_view _address) :
        AssetInfo(AssetType::Image, _address)
    {
    }
    ImageInfo(const ImageInfo&) = default;
    ImageInfo(ImageInfo&&) = default;
    ImageInfo& operator=(const ImageInfo&) = default;
    ImageInfo& operator=(ImageInfo&&) = default;
    ColorSpace colorSpace = ColorSpace::SRGB;
    virtual AssetInfo* Clone() const override
    {
        return new ImageInfo(*this);
    }
};
template <>
struct ToJsonImpl<ColorSpace>
{
    Json operator()(const ColorSpace& t)
    {
        switch (t)
        {
        case ColorSpace::SRGB:
            return "SRGB";
        case ColorSpace::LINEAR:
            return "LINEAR";
        default:
            return "Unknown";
        }
    }
};
template <>
struct FromJsonImpl<ColorSpace>
{
    std::expected<ColorSpace, std::string> operator()(const Json& json)
    {
        std::string s = json.get<std::string>();
        if (s == "SRGB")
        {
            return ColorSpace::SRGB;
        }
        else if (s == "LINEAR")
        {
            return ColorSpace::LINEAR;
        }
        else
        {
            return std::unexpected<std::string>(std::string("Unknown ColorSpace:") + s);
        }
    }
};
template <>
struct ToJsonImpl<ImageInfo>
{
    Json operator()(const ImageInfo& t)
    {
        Json j;
        j["type"] = ToJson(t.type);
        j["address"] = t.address;
        j["colorSpace"] = ToJson(t.colorSpace);
        return j;
    }
};
template <>
struct FromJsonImpl<ImageInfo>
{
    std::expected<ImageInfo, std::string> operator()(const Json& json)
    {
        if (!json.is_object())
        {
            return std::unexpected<std::string>("ImageInfo should be an object");
        }
        // check type
        auto json_type_iter = json.find("type");
        if (json_type_iter == json.end())
        {
            return std::unexpected<std::string>("ImageInfo should have type field");
        }
        auto type = FromJson<AssetType>(json["type"]);
        if (!type)
        {
            return std::unexpected<std::string>(std::format("failed to get type: {}", type.error()));
        }
        if (type.value() != AssetType::Image)
        {
            return std::unexpected<std::string>("type should be Image");
        }

        // get address
        auto json_address_iter = json.find("address");
        if (json_address_iter == json.end())
        {
            return std::unexpected<std::string>("ImageInfo should have address field");
        }
        ImageInfo info(FromJson<std::string>(json["address"]).value());
        // get colorSpace
        auto json_colorSpace_iter = json.find("colorSpace");
        if (json_colorSpace_iter == json.end())
        {
            return std::unexpected<std::string>("ImageInfo should have colorSpace field");
        }
        auto colorSpace = FromJson<ColorSpace>(json["colorSpace"]);
        if (!colorSpace)
        {
            return std::unexpected<std::string>(std::format("failed to get colorSpace: {}", colorSpace.error()));
        }
        info.colorSpace = colorSpace.value();
        return info;
    }
};
} // namespace Aether::Resource

namespace Aether
{
template <>
struct ToJson<Resource::ColorSpace>
{
    Json operator()(const Resource::ColorSpace& t)
    {
        switch (t)
        {
        case Resource::ColorSpace::SRGB:
            return "SRGB";
        case Resource::ColorSpace::LINEAR:
            return "LINEAR";
        default:
            return "Unknown";
        }
    }
};
template <>
struct FromJson<Resource::ColorSpace>
{
    std::expected<Resource::ColorSpace, std::string> operator()(const Json& json)
    {
        std::string s = json.get<std::string>();
        if (s == "SRGB")
        {
            return Resource::ColorSpace::SRGB;
        }
        else if (s == "LINEAR")
        {
            return Resource::ColorSpace::LINEAR;
        }
        else
        {
            return std::unexpected<std::string>(std::string("Unknown ColorSpace:") + s);
        }
    }
};
} // namespace Aether