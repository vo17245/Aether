#pragma once
#include "AssetInfo.h"
namespace Aether::Resource
{

struct ShaderInfo : public AssetInfo
{
    ShaderInfo(const std::string_view _address) :
        AssetInfo(AssetType::Shader, _address)
    {
    }
    enum class ShaderLanguage
    {
        GLSL,
        HLSL,
        SPIRV,
    };
    ShaderLanguage language = ShaderLanguage::GLSL;
    enum class ShaderStageType
    {
        Vertex,
        Fragment,
        Compute,
    };
    ShaderStageType stage = ShaderStageType::Vertex;
    std::string entryPoint = "main";
    virtual AssetInfo* Clone()const override
    {
        return new ShaderInfo(*this);
    }
};
template <>
struct ToJsonImpl<ShaderInfo::ShaderLanguage>
{
    Json operator()(const ShaderInfo::ShaderLanguage& t)
    {
        switch (t)
        {
        case ShaderInfo::ShaderLanguage::GLSL:
            return "GLSL";
        case ShaderInfo::ShaderLanguage::HLSL:
            return "HLSL";
        case ShaderInfo::ShaderLanguage::SPIRV:
            return "SPIRV";
        default:
            return "Unknown";
        }
    }
};
template <>
struct FromJsonImpl<ShaderInfo::ShaderLanguage>
{
    std::expected<ShaderInfo::ShaderLanguage, std::string> operator()(const Json& json)
    {
        std::string s = json.get<std::string>();
        if (s == "GLSL")
        {
            return ShaderInfo::ShaderLanguage::GLSL;
        }
        else if (s == "HLSL")
        {
            return ShaderInfo::ShaderLanguage::HLSL;
        }
        else if (s == "SPIRV")
        {
            return ShaderInfo::ShaderLanguage::SPIRV;
        }
        else
        {
            return std::unexpected<std::string>(std::string("Unknown ShaderLanguage:") + s);
        }
    }
};
template <>
struct ToJsonImpl<ShaderInfo::ShaderStageType>
{
    Json operator()(const ShaderInfo::ShaderStageType& t)
    {
        switch (t)
        {
        case ShaderInfo::ShaderStageType::Vertex:
            return "Vertex";
        case ShaderInfo::ShaderStageType::Fragment:
            return "Fragment";
        case ShaderInfo::ShaderStageType::Compute:
            return "Compute";
        default:
            return "Unknown";
        }
    }
};
template <>
struct FromJsonImpl<ShaderInfo::ShaderStageType>
{
    std::expected<ShaderInfo::ShaderStageType, std::string> operator()(const Json& json)
    {
        std::string s = json.get<std::string>();
        if (s == "Vertex")
        {
            return ShaderInfo::ShaderStageType::Vertex;
        }
        else if (s == "Fragment")
        {
            return ShaderInfo::ShaderStageType::Fragment;
        }
        else if (s == "Compute")
        {
            return ShaderInfo::ShaderStageType::Compute;
        }
        else
        {
            return std::unexpected<std::string>(std::string("Unknown ShaderStageType:") + s);
        }
    }
};
template <>
struct ToJsonImpl<ShaderInfo>
{
    Json operator()(const ShaderInfo& t)
    {
        Json j;
        j["type"] = ToJson(t.type);
        j["address"] = t.address;
        j["language"] = ToJson(t.language);
        j["stage"] = ToJson(t.stage);
        j["entryPoint"] = t.entryPoint;
        return j;
    }
};
template <>
struct FromJsonImpl<ShaderInfo>
{
    std::expected<ShaderInfo, std::string> operator()(const Json& json)
    {
        if (!json.is_object())
        {
            return std::unexpected<std::string>("ShaderInfo should be an object");
        }
        // check type
        auto json_type_iter = json.find("type");
        if (json_type_iter == json.end())
        {
            return std::unexpected<std::string>("ShaderInfo should have type field");
        }
        auto type = FromJson<AssetType>(json["type"]);
        if (!type)
        {
            return std::unexpected<std::string>(std::format("failed to get type: {}", type.error()));
        }
        if (type.value() != AssetType::Shader)
        {
            return std::unexpected<std::string>("type should be Shader");
        }

        // get address
        auto json_address_iter = json.find("address");
        if (json_address_iter == json.end())
        {
            return std::unexpected<std::string>("ShaderInfo should have address field");
        }
        ShaderInfo info(FromJson<std::string>(json["address"]).value());
        // get language
        auto json_language_iter = json.find("language");
        if (json_language_iter == json.end())
        {
            return std::unexpected<std::string>("ShaderInfo should have language field");
        }
        auto language = FromJson<ShaderInfo::ShaderLanguage>(json["language"]);
        if (!language)
        {
            return std::unexpected<std::string>(std::format("failed to get language: {}", language.error()));
        }
        info.language = language.value();
        // get stage
        auto json_stage_iter = json.find("stage");
        if (json_stage_iter == json.end())
        {
            return std::unexpected<std::string>("ShaderInfo should have stage field");
        }
        auto stage = FromJson<ShaderInfo::ShaderStageType>(json["stage"]);
        if (!stage)
        {
            return std::unexpected<std::string>(std::format("failed to get stage: {}", stage.error()));
        }
        info.stage = stage.value();
        // get entryPoint
        auto json_entryPoint_iter = json.find("entryPoint");
        if (json_entryPoint_iter == json.end())
        {
            return std::unexpected<std::string>("ShaderInfo should have entryPoint field");
        }
        info.entryPoint = json["entryPoint"].get<std::string>();
        return info;
    }
};
} // namespace Aether::Resource