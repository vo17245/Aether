#pragma once
#include <Core/Core.h>
#include <Debug/Log.h>
namespace Aether::Project
{
enum class AssetType
{
    Texture,
    Mesh,
    Unknown,
};
inline const char* AssetTypeToString(AssetType type)
{
    switch (type)
    {
    case AssetType::Texture:
        return "Texture";
    case AssetType::Mesh:
        return "Mesh";
    case AssetType::Unknown:
        return "Unknown";
    default:
        LogE("AssetTypeToString: Unknown asset type enum value: {}", static_cast<int>(type));
        return "Unknown";
    }
}
inline AssetType AssetTypeFromString(const std::string& typeStr)
{
    if (typeStr == "Texture")
    {
        return AssetType::Texture;
    }
    else if (typeStr == "Mesh")
    {
        return AssetType::Mesh;
    }
    else if (typeStr == "Unknown")
    {
        return AssetType::Unknown;
    }
    else
    {
        LogE("AssetTypeFromString: Unknown asset type string: {}", typeStr);
        return AssetType::Unknown;
    }
}
} // namespace Aether::Project