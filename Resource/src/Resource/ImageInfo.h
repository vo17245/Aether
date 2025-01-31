#pragma once
#include "AssetInfo.h"
namespace Aether::Resource
{
struct ImageInfo : public AssetInfo
{
    enum class ColorSpace
    {
        SRGB,
        LINEAR,
    };
    ImageInfo(const Address& _address) :
        AssetInfo(AssetType::Image, _address)
    {
    }
    ImageInfo(const ImageInfo&) = default;
    ImageInfo(ImageInfo&&) = default;
    ImageInfo& operator=(const ImageInfo&) = default;
    ImageInfo& operator=(ImageInfo&&) = default;
    ColorSpace colorSpace = ColorSpace::SRGB;
};
template <>
struct ToJsonImpl<ImageInfo::ColorSpace>
{
    Json operator()(const ImageInfo::ColorSpace& t)
    {
        switch (t)
        {
        case ImageInfo::ColorSpace::SRGB:
            return "SRGB";
        case ImageInfo::ColorSpace::LINEAR:
            return "LINEAR";
        default:
            return "Unknown";
        }
    }
};
template <>
struct FromJsonImpl<ImageInfo::ColorSpace>
{
    std::expected<ImageInfo::ColorSpace, std::string> operator()(const Json& json)
    {
        std::string s = json.get<std::string>();
        if (s == "SRGB")
        {
            return ImageInfo::ColorSpace::SRGB;
        }
        else if (s == "LINEAR")
        {
            return ImageInfo::ColorSpace::LINEAR;
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
        j["address"] = t.address.GetAddress();
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
        ImageInfo info(FromJson<Address>(json["address"]).value());
        // get colorSpace
        auto json_colorSpace_iter = json.find("colorSpace");
        if (json_colorSpace_iter == json.end())
        {
            return std::unexpected<std::string>("ImageInfo should have colorSpace field");
        }
        auto colorSpace = FromJson<ImageInfo::ColorSpace>(json["colorSpace"]);
        if (!colorSpace)
        {
            return std::unexpected<std::string>(std::format("failed to get colorSpace: {}", colorSpace.error()));
        }
        info.colorSpace = colorSpace.value();
        return info;
    }
};
} // namespace Aether::Resource