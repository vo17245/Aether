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
AETHER_NAMESPACE_BEGIN
class ModelAsset 
{
public:
    ModelAsset(){}
    /*  函数   */
    ModelAsset(const std::string& path)
        :m_Path(path)
    {
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
};
AETHER_NAMESPACE_END