#include "ModelAsset.h"
#include <iostream>



AETHER_NAMESPACE_BEGIN
void ModelAsset::LoadModel(const std::string& path)
{
    Assimp::Importer import;
    const aiScene * scene = import.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);
    //const aiScene * scene = import.ReadFile(path, aiProcess_Triangulate);
    assert(!(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) && "加载模型失败");
    ProcessNode(scene->mRootNode, scene);
    

}

void ModelAsset::ProcessNode(aiNode* node, const aiScene* scene)
{
    // 处理节点所有的网格（如果有的话）
    for (size_t i = 0; i < node->mNumMeshes; i++)
    {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        
        m_Meshes.push_back(ProcessMesh(mesh, scene));
    }
    
    // 接下来对它的子节点重复这一过程
    for (size_t i = 0; i < node->mNumChildren; i++)
    {
        ProcessNode(node->mChildren[i], scene);
    }
}

MeshAsset ModelAsset::ProcessMesh(aiMesh* mesh, const aiScene* scene)
{
    MeshAsset myMesh;
    for (size_t i = 0; i < mesh->mNumVertices; i++)
    {
        Vertex vertex;
        //位置
        vertex.Position.x() = mesh->mVertices[i].x;
        vertex.Position.y() = mesh->mVertices[i].y;
        vertex.Position.z() = mesh->mVertices[i].z; 
        //法向量
        vertex.Normal.x() = mesh->mNormals[i].x;
        vertex.Normal.y() = mesh->mNormals[i].y;
        vertex.Normal.z() = mesh->mNormals[i].z;
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
    //if (mesh->mMaterialIndex >= 0)
    //{
    //    aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
    //    auto diffuseTextures = LoadMaterialTextures(scene,material,
    //        aiTextureType_DIFFUSE, "texture_diffuse");
    //    myMesh.Textures.insert(myMesh.Textures.end(),diffuseTextures.end(), //diffuseTextures.end());
    //    auto specularTextures = LoadMaterialTextures(scene,material,
    //        aiTextureType_SPECULAR, "texture_specular");
    //    myMesh.Textures.insert(myMesh.Textures.end(), specularTextures.end(), //specularTextures.end());
    //}

    return myMesh;
}

std::vector<std::shared_ptr<TextureAsset>> ModelAsset::LoadMaterialTextures(const aiScene* scene,aiMaterial* mat, aiTextureType type, const std::string& typeName)
{
    std::vector<std::shared_ptr<TextureAsset>> textures;

    for (unsigned int i = 0; i < mat->GetTextureCount(type); i++)
    {
        aiString str;
        mat->GetTexture(type, i, &str);
        auto texture = std::make_shared<TextureAsset>(str.C_Str(), typeName);
        textures.push_back(texture);
    }
    return textures;
}


AETHER_NAMESPACE_END