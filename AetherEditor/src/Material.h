#pragma once
#include <Render/Render.h>
#include <Core/Core.h>
#include "FileWatchListener.h"
namespace Aether
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
    float min = 0.0f;
    float max=1.0f;
};
struct MaterialVec2 : public MaterialData
{
    MaterialVec2() : MaterialData(MaterialDataType::Vec2)
    {
    }
    Vec2f value{0.0f, 0.0f};
    Vec2f min{0.0f, 0.0f};
    Vec2f max{1.0f, 1.0f};
};
struct MaterialVec3 : public MaterialData
{
    MaterialVec3() : MaterialData(MaterialDataType::Vec3)
    {
    }
    Vec3f value{0.0f, 0.0f, 0.0f};
    Vec3f min{0.0f, 0.0f, 0.0f};
    Vec3f max{1.0f, 1.0f, 1.0f};
};
struct MaterialVec4 : public MaterialData
{
    MaterialVec4() : MaterialData(MaterialDataType::Vec4)
    {
    }
    Vec4f value{0.0f, 0.0f, 0.0f, 0.0f};
    Vec4f min{0.0f, 0.0f, 0.0f, 0.0f};
    Vec4f max{1.0f, 1.0f, 1.0f, 1.0f};
};
struct Keyword
{
    std::string name;
    uint32_t   flag;
};
using KeywordFlags=uint32_t;
struct Material
{
    std::string dir;
    std::string name;
    std::string vertPath;
    std::string fragPath;
    std::vector<Scope<MaterialData>> datas;
    std::vector<Keyword> keywords;
};
} // namespace Aether