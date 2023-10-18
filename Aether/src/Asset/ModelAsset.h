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
class ModelAsset 
{
public:
    ModelAsset(){}
    /*  函数   */
    ModelAsset(const std::string& path)
        :m_Path(path)
    {
        Log::Debug("EmbeddedTextureCache size {0}", m_EmbeddedTextureCache.size());
        LoadModel(path);
        
    } 
public:
    inline const std::vector<MeshAsset>& GetMeshes()const { return m_Meshes; }
    inline  std::vector<MeshAsset>& GetMeshes() { return m_Meshes; }
    inline const std::string& GetPath() { return m_Path; }
    inline void PushMesh(MeshAsset&& mesh) { m_Meshes.push_back(std::move(mesh)); }
    inline void PushMesh(const MeshAsset& mesh) { m_Meshes.push_back(mesh); }
private:
    /*  模型数据  */
    std::vector<MeshAsset> m_Meshes;
    std::string m_Path;
    /*  函数   */
    void LoadModel(const std::string& path);
    void ProcessNode(aiNode *node, const aiScene *scene);
    MeshAsset ProcessMesh(aiMesh *mesh, const aiScene *scene);
    std::vector<std::shared_ptr<TextureAsset>> LoadMaterialTextures(const aiScene* scene,aiMaterial *mat, aiTextureType type, const std::string& typeName);
private:
    std::unordered_map<size_t,Ref<TextureAsset>> m_EmbeddedTextureCache;
    std::unordered_map<std::string, Ref<TextureAsset>> m_FileTextureCache;
    std::optional<Ref<TextureAsset>> LoadEmbeddedTexture(size_t index,aiTexture** textures,const std::string& typeName);
    std::optional<Ref<TextureAsset>> LoadFileTexture(const std::string& path, const std::string& typeName);

};
AETHER_NAMESPACE_END