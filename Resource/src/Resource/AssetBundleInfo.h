#pragma once
#include "AssetInfo.h"
#include "AssetInfoSerialization.h"
#include "ToJson.h"
namespace Aether::Resource
{
    struct AssetBundleInfo
    {
        std::unordered_map<std::string, Scope<AssetInfo>> assets;
    };
    template<>
    struct ToJsonImpl<AssetBundleInfo>
    {
        Json operator()(const AssetBundleInfo& t)
        {
            Json json_assets = Json::object();
            for (const auto& [address, asset] : t.assets)
            {
                json_assets[address] = ToJson(asset.get());
            }
            Json res;
            res["assets"] = std::move(json_assets);
            return res;
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
            // get assets
            auto json_assets_iter = json.find("assets");
            if (json_assets_iter == json.end())
            {
                return std::unexpected<std::string>("AssetBundleInfo should has assets filed");
            }
            auto& json_assets = json["assets"];
            AssetBundleInfo res;
            for (const auto& [key, value] : json_assets.items())
            {
                auto address = key;
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