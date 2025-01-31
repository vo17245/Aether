#pragma once
#include "AssetInfo.h"
#include "AssetInfoSerialization.h"
#include "ToJson.h"
namespace Aether::Resource
{
    struct AssetBundleInfo
    {
        std::unordered_map<Address, Scope<AssetInfo>, AddressHash> assets;
    };
    template<>
    struct ToJsonImpl<AssetBundleInfo>
    {
        Json operator()(const AssetBundleInfo& t)
        {
            Json json = Json::object();
            for (const auto& [address, asset] : t.assets)
            {
                json[address.GetAddress()] = ToJson(asset);
            }
            return json;
        }
    };
    template<>
    struct FromJsonImpl<AssetBundleInfo>
    {
        std::expected<AssetBundleInfo, std::string> operator()(const Json& json)
        {
            if (!json.is_object())
            {
                return std::unexpected<std::string>("AssetBundleInfo should be an object");
            }
            AssetBundleInfo res;
            for (const auto& [key, value] : json.items())
            {
                auto address = Address(key);
                auto asset = FromJson<AssetInfo*>(value);
                if (!asset)
                {
                    return std::unexpected<std::string>(asset.error());
                }
                res.assets[address] = Scope<AssetInfo>(asset.value());
            }
            return res;
        }
    };
   
}