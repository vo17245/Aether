#pragma once
#include <Render/Render.h>
#include <Core/Core.h>
namespace Aether::URP
{
enum class MaterialDataType
{
    Float,
    Vec2,
    Vec3,
    Vec4,
    Texture2D
};
struct MaterialData
{
    MaterialData(MaterialDataType _type) : type(_type)
    {
    }
    std::string name;
    MaterialDataType type;
};
struct MaterialTexture2D : public MaterialData
{
    MaterialTexture2D() : MaterialData(MaterialDataType::Texture2D)
    {
    }
    std::string path;
    RenderGraph::ResourceId<DeviceTexture> texture;
};
struct MaterialFloat : public MaterialData
{
    MaterialFloat() : MaterialData(MaterialDataType::Float)
    {
    }
    float value = 0.0f;
};
struct MaterialVec2 : public MaterialData
{
    MaterialVec2() : MaterialData(MaterialDataType::Vec2)
    {
    }
    Vec2f value{0.0f, 0.0f};
};
struct MaterialVec3 : public MaterialData
{
    MaterialVec3() : MaterialData(MaterialDataType::Vec3)
    {
    }
    Vec3f value{0.0f, 0.0f, 0.0f};
};
struct MaterialVec4 : public MaterialData
{
    MaterialVec4() : MaterialData(MaterialDataType::Vec4)
    {
    }
    Vec4f value{0.0f, 0.0f, 0.0f, 0.0f};
};
struct Keyword
{
    std::string name;
    uint32_t   flag;
};
using KeywordFlags=uint32_t;
struct MaterialComponent
{
    std::string name;
    std::string vertPath;
    std::string fragPath;
    std::vector<Scope<MaterialData>> datas;
    std::vector<Keyword> keywords;
};

} // namespace Aether::URP
namespace Aether
{
template <>
struct ToJson<URP::MaterialData>
{
    Json operator()(const URP::MaterialData& data) const
    {
        Json json;
        json["name"] = data.name;
        json["type"] = static_cast<int>(data.type);
        switch (data.type)
        {
        case URP::MaterialDataType::Float: {
            auto& d = static_cast<const URP::MaterialFloat&>(data);
            json["value"] = d.value;
            break;
        }
        case URP::MaterialDataType::Vec2: {
            auto& d = static_cast<const URP::MaterialVec2&>(data);
            json["value"] = ToJson<Vec2f>()(d.value);
            break;
        }
        case URP::MaterialDataType::Vec3: {
            auto& d = static_cast<const URP::MaterialVec3&>(data);
            json["value"] = ToJson<Vec3f>()(d.value);
            break;
        }
        case URP::MaterialDataType::Vec4: {
            auto& d = static_cast<const URP::MaterialVec4&>(data);
            json["value"] = ToJson<Vec4f>()(d.value);
            break;
        }
        case URP::MaterialDataType::Texture2D: {
            auto& d = static_cast<const URP::MaterialTexture2D&>(data);
            json["path"] = d.path;
            break;
        }
        }
        return json;
    }
};
template <>
struct FromJson<URP::MaterialData>
{
    Scope<URP::MaterialData> operator()(const Json& json) const
    {
        auto type = static_cast<URP::MaterialDataType>(json["type"].get<int>());
        Scope<URP::MaterialData> data;
        switch (type)
        {
        case URP::MaterialDataType::Float: {
            auto d = CreateScope<URP::MaterialFloat>();
            d->name = json["name"].get<std::string>();
            d->value = json["value"].get<float>();
            data = std::move(d);
            break;
        }
        case URP::MaterialDataType::Vec2: {
            auto d = CreateScope<URP::MaterialVec2>();
            d->name = json["name"].get<std::string>();
            d->value = (Vec2f)FromJson<Vec2f>()(json["value"])->value();
            data = std::move(d);
            break;
        }
        case URP::MaterialDataType::Vec3: {
            auto d = CreateScope<URP::MaterialVec3>();
            d->name = json["name"].get<std::string>();
            d->value = (Vec3f)FromJson<Vec3f>()(json["value"])->value();
            data = std::move(d);
            break;
        }
        case URP::MaterialDataType::Vec4: {
            auto d = CreateScope<URP::MaterialVec4>();
            d->name = json["name"].get<std::string>();
            d->value = (Vec4f)FromJson<Vec4f>()(json["value"])->value();
            data = std::move(d);
            break;
        }
        case URP::MaterialDataType::Texture2D: {
            auto d = CreateScope<URP::MaterialTexture2D>();
            d->name = json["name"].get<std::string>();
            d->path = json["path"].get<std::string>();
            data = std::move(d);
            break;
        }
        }
        return data;
    }
};
template<>
struct ToJson<URP::MaterialComponent>
{
    Json operator()(const URP::MaterialComponent& material) const
    {
        Json json;
        json["name"] = material.name;
        json["vertPath"] = material.vertPath;
        json["fragPath"] = material.fragPath;
        Json dataJson = Json::array();
        for (const auto& data : material.datas)
        {
            dataJson.push_back(ToJson<URP::MaterialData>()(*data));
        }
        json["datas"] = dataJson;
        json["keywords"] = Json::array();
        for (const auto& kw : material.keywords)
        {
            Json kwJson;
            kwJson["name"] = kw.name;
            kwJson["flag"] = kw.flag;
            json["keywords"].push_back(kwJson);
        }

        return json;
    }
};
template<>
struct FromJson<URP::MaterialComponent>
{
    std::expected<URP::MaterialComponent, std::string> operator()(const Json& json) const
    {
        URP::MaterialComponent material;
        material.name = json["name"].get<std::string>();
        material.vertPath = json["vertPath"].get<std::string>();
        material.fragPath = json["fragPath"].get<std::string>();
        auto& dataJson = json["datas"];
        if (!dataJson.is_array())
        {
            return std::unexpected<std::string>("datas is not an array");
        }
        for (const auto& item : dataJson)
        {
            material.datas.push_back(FromJson<URP::MaterialData>()(item));
        }
        auto& kwJson = json["keywords"];
        if (!kwJson.is_array())
        {
            return std::unexpected<std::string>("keywords is not an array");
        }
        for (const auto& item : kwJson)
        {
            URP::Keyword kw;
            kw.name = item["name"].get<std::string>();
            kw.flag = item["flag"].get<uint32_t>();
            material.keywords.push_back(kw);
        }
        
        return material;
    }

};
} // namespace Aether