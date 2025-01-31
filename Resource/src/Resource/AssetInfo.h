#pragma once
#include "Address.h"
#include <IO/Json.h>
#include <unordered_map>
#include "Core/Base.h"
#include "ToJson.h"
#include "FromJson.h"
namespace Aether::Resource
{

enum class AssetType
{
    Image,
    Shader,
    Unknown,
};
template <>
struct ToJsonImpl<AssetType>
{
    Json operator()(const AssetType& t)
    {
        switch (t)
        {
        case AssetType::Image:
            return "Image";
        case AssetType::Shader:
            return "Shader";
        default:
            return "Unknown";
        }
    }
};
template <>
struct FromJsonImpl<AssetType>
{
    std::expected<AssetType, std::string> operator()(const Json& json)
    {
        std::string str = json.get<std::string>();
        if (str == "Image")
        {
            return AssetType::Image;
        }
        else if (str == "Shader")
        {
            return AssetType::Shader;
        }
        else
        {
            return std::unexpected<std::string>(std::string("Unknown AssetType:") + str);
        }
    }
};
struct AssetInfo
{
    AssetType type;
    Address address;
    AssetInfo(AssetType _type, const Address& _address) :
        type(_type), address(_address)
    {
    }
    AssetInfo(const AssetInfo&) = default;
    AssetInfo(AssetInfo&&) = default;
    AssetInfo& operator=(const AssetInfo&) = default;
    AssetInfo& operator=(AssetInfo&&) = default;
};

} // namespace Aether::Resource