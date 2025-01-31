#pragma once
#include "AssetInfo.h"
#include "ShaderInfo.h"
#include "ImageInfo.h"
namespace Aether::Resource
{
template <>
struct ToJsonImpl<AssetInfo*>
{
    Json operator()(const AssetInfo* t)
    {
        switch (t->type)
        {
        case AssetType::Image:
            return ToJson(*(const ImageInfo*)(t));
        case AssetType::Shader:
            return ToJson(*(const ShaderInfo*)(t));
        default:
            return "Unknown AssetType";
        }
    }
};
template <>
struct FromJsonImpl<AssetInfo*>
{
    std::expected<AssetInfo*, std::string> operator()(const Json& json)
    {
        if (!json.is_object())
        {
            return std::unexpected<std::string>("AssetInfo should be an object");
        }
        // get type
        auto json_type_iter = json.find("type");
        if (json_type_iter == json.end())
        {
            return std::unexpected<std::string>("AssetInfo should have type");
        }
        auto type = FromJson<AssetType>(json["type"]);
        if (!type)
        {
            return std::unexpected<std::string>(std::format("failed to get type: {}", type.error()));
        }
        switch (type.value())
        {
        case AssetType::Image: {
            auto imageInfoEx=FromJson<ImageInfo>(json);
            if(!imageInfoEx)
            {
                return std::unexpected<std::string>(imageInfoEx.error());
            }
            return new ImageInfo(std::move(imageInfoEx.value()));
        }
        break;

        case AssetType::Shader:
        {
            auto shaderInfoEx=FromJson<ShaderInfo>(json);
            if(!shaderInfoEx)
            {
                return std::unexpected<std::string>(shaderInfoEx.error());
            }
            return new ShaderInfo(std::move(shaderInfoEx.value()));
        }
        default:
            return std::unexpected<std::string>("Unknown AssetType");
        }
    }
};
} // namespace Aether::Resource