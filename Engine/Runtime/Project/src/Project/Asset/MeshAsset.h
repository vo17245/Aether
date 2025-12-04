#pragma once
#include "Asset.h"
namespace Aether::Project
{
class MeshAsset : public Asset
{
public:
    MeshAsset() : Asset(AssetType::Mesh)
    {
    }
private:
    std::string m_FilePath;
};
} // namespace Aether::Project