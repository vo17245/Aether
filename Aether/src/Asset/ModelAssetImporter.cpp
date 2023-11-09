#include "ModelAssetImporter.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
AETHER_NAMESPACE_BEGIN
std::optional<std::vector<std::shared_ptr<TextureAsset>>>
ModelAssetImporter::LoadMaterialTextures(ModelAsset& modelAsset,const aiScene* scene, aiMaterial* mat, aiTextureType type, const std::string& typeName)
{
    std::vector<std::shared_ptr<TextureAsset>> textures;
    for (unsigned int i = 0; i < mat->GetTextureCount(type); i++)
    {
        aiString str;
        mat->GetTexture(type, i, &str);
        if (str.C_Str()[0] == '*')
        {
            //load from embedded image data
            if (scene->mTextures == nullptr)
            {
                debug_log("scene->mTextures == nullptr");
                return std::nullopt;
            }
            uint32_t textureIndex = atoi(str.C_Str() + 1);
            if (!(textureIndex >= 0 && textureIndex < scene->mNumTextures))
            {
                debug_log("!(textureIndex >= 0 && textureIndex < scene->mNumTextures)");
                return std::nullopt;

            }
            auto loadres = modelAsset.LoadEmbeddedTexture(textureIndex, scene->mTextures, typeName);
            if (loadres)
            {
                textures.push_back(loadres.value());
            }
            else
            {
                debug_log("failed to load embedded texture");
                return std::nullopt;
            }
                
        }
        else
        {
            //load from filesystem
            auto loadRes=modelAsset.LoadFileTexture(str.C_Str(), typeName);
            
            if (!loadRes)
            {
                debug_log("failed to load texture");
                return std::nullopt;
            }
            textures.push_back(loadRes.value());
        }
    }
    return textures;
}
bool ModelAssetImporter::ProcessMesh(ModelAsset& modelAsset, aiMesh* mesh, const aiScene* scene)
{
    MeshAsset myMesh;
    for (size_t i = 0; i < mesh->mNumVertices; i++)
    {
        Vertex vertex;
        //位置
        if (mesh->HasPositions())
        {
            vertex.Position.x() = mesh->mVertices[i].x;
            vertex.Position.y() = mesh->mVertices[i].y;
            vertex.Position.z() = mesh->mVertices[i].z;
        }
        
        //法向量
        if (mesh->HasNormals())
        {
            vertex.Normal.x() = mesh->mNormals[i].x;
            vertex.Normal.y() = mesh->mNormals[i].y;
            vertex.Normal.z() = mesh->mNormals[i].z;
        }
        
        //纹理坐标

        if (mesh->mTextureCoords[0]) // 网格是否有纹理坐标？
        {
            vertex.TexCoords.x() = mesh->mTextureCoords[0][i].x;
            vertex.TexCoords.y() = mesh->mTextureCoords[0][i].y;
        }
        else
        {
            vertex.TexCoords.x() = 0;
            vertex.TexCoords.y() = 0;
        }
        myMesh.Vertices.emplace_back(vertex);

    }
    // 处理索引
    for (size_t i = 0; i < mesh->mNumFaces; i++)
    {
        aiFace face = mesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; j++)
            myMesh.Indices.push_back(face.mIndices[j]);

    }

    // 处理材质
    if (mesh->mMaterialIndex >= 0)
    {
        aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
        auto diffuseTextures = LoadMaterialTextures(modelAsset,scene, material,
            aiTextureType_DIFFUSE, "texture_diffuse");
        if (!diffuseTextures)
        {
            debug_log("failed to load diffuseTextures");
            return false;
        }
        myMesh.Textures.insert(myMesh.Textures.end(), diffuseTextures.value().begin(), diffuseTextures.value().end());
        
        auto specularTextures = LoadMaterialTextures(modelAsset,scene, material,
            aiTextureType_SPECULAR, "texture_specular");
        if (!specularTextures)
        {
            debug_log("failed to load specularTextures");
            return false;
        }
        myMesh.Textures.insert(myMesh.Textures.end(), specularTextures.value().begin(), specularTextures.value().end());
    }
    modelAsset.m_Meshes.push_back(std::move(myMesh));

    return true;
}
bool ModelAssetImporter::ProcessNode(ModelAsset& modelAsset, aiNode* node, const aiScene* scene)
{
    // 处理节点所有的网格（如果有的话）
    for (size_t i = 0; i < node->mNumMeshes; i++)
    {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        bool ret=ProcessMesh(modelAsset, mesh, scene);
        if (!ret)return false;
    }

    // 接下来对它的子节点重复这一过程
    for (size_t i = 0; i < node->mNumChildren; i++)
    {
        bool ret=ProcessNode(modelAsset,node->mChildren[i], scene);
        if (!ret)return false;
    }
    return true;
}
std::optional<Ref<ModelAsset>> ModelAssetImporter::LoadFromFile(const std::string& path)
{
    Assimp::Importer import;
    const aiScene* scene = import.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);
    if ((!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode))
    {
        debug_log("Failed to load model,model file path {0}",path);
        return std::nullopt;
    }
    auto modelAsset=CreateRef<ModelAsset>();
    bool ret=ProcessNode(*modelAsset, scene->mRootNode, scene);
    if (!ret)
    {
        debug_log("Failed to load node");
        return std::nullopt;
    }
	return modelAsset;
}
AETHER_NAMESPACE_END


