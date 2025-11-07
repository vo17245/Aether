#pragma once
#include <Project/Project.h>

namespace Aether::Project
{
class TextureAsset : public Asset
{
public:
    TextureAsset() : Asset(AssetType::Texture)
    {
    }
public:
    bool sRGB = true;
};
struct ImportTextureParams
{
    std::string FilePath;
    std::string Address;
    std::string Name;
    bool sRGB = true;
};
/**
 * @return error msg if failed,otherwise return nullopt
 */
std::optional<std::string> ImportTexture(Project& project, ImportTextureParams& params);
} // namespace Aether::Project