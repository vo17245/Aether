#pragma once

#include <vector>
#include "../Render/Shader.h"
#include <string>
#include "MeshAsset.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "../Render/Texture2D.h"
#include <memory>
#include "TextureAsset.h"
#include <unordered_map>
#include "../Core/Log.h"
AETHER_NAMESPACE_BEGIN
class ModelAssetImporter;
class ModelAsset 
{
    friend class ModelAssetImporter;
public:
    ModelAsset(){}
 
    
public:
    inline const std::vector<MeshAsset>& GetMeshes()const { return m_Meshes; }
    inline  std::vector<MeshAsset>& GetMeshes() { return m_Meshes; }
private:
    /*  模型数据  */
    std::vector<MeshAsset> m_Meshes;
private:
    std::unordered_map<size_t,Ref<TextureAsset>> m_EmbeddedTextureCache;
    std::unordered_map<std::string, Ref<TextureAsset>> m_FileTextureCache;
    std::optional<Ref<TextureAsset>> LoadEmbeddedTexture(size_t index,aiTexture** textures,const std::string& typeName);
    std::optional<Ref<TextureAsset>> LoadFileTexture(const std::string& path, const std::string& typeName);
};

AETHER_NAMESPACE_END