#pragma once
#include "ModelAsset.h"
#include "../Core/Core.h"
namespace Aether{
class ModelAssetImporter
{
public:
    static std::optional<Ref<ModelAsset>> LoadFromFile(const std::string& path);
    static bool ProcessNode(ModelAsset& modelAsset, aiNode* node, const aiScene* scene);
    static bool ProcessMesh(ModelAsset& modelAsset, aiMesh* mesh, const aiScene* scene);
   

    static std::optional<std::vector<std::shared_ptr<TextureAsset>>>
    LoadMaterialTextures(ModelAsset& modelAsset, const aiScene* scene, aiMaterial* mat, aiTextureType type, const std::string& typeName);
};
}//namespace Aether