#include "ModelAsset.h"
#include <iostream>
#include "../Core/Log.h"


AETHER_NAMESPACE_BEGIN
void ModelAsset::LoadModel(const std::string& path)
{
    Log::Debug("ModelAsset::LoadModel(const std::string& path)");
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
    if (mesh->mMaterialIndex >= 0)
    {
        aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
        auto diffuseTextures = LoadMaterialTextures(scene,material,
            aiTextureType_DIFFUSE, "texture_diffuse");
        myMesh.Textures.insert(myMesh.Textures.end(),diffuseTextures.begin(), diffuseTextures.end());
        auto specularTextures = LoadMaterialTextures(scene,material,
            aiTextureType_SPECULAR, "texture_specular");
        myMesh.Textures.insert(myMesh.Textures.end(), specularTextures.begin(), specularTextures.end());
    }

    return myMesh;
}

std::vector<std::shared_ptr<TextureAsset>> ModelAsset::LoadMaterialTextures(const aiScene* scene,aiMaterial* mat, aiTextureType type, const std::string& typeName)
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
                Log::Error("scene->mTextures == nullptr {0}:{1}", __FILE__, __LINE__);
                continue;
            }
            uint32_t textureIndex = atoi(str.C_Str()+1);
            if (!(textureIndex >= 0 && textureIndex < scene->mNumTextures))
            {
                Log::Error("!(textureIndex >= 0 && textureIndex < scene->mNumTextures) {0}:{1}", __FILE__, __LINE__);
                continue;
                
            }
            auto loadres=LoadEmbeddedTexture(textureIndex, scene->mTextures, typeName);
            if(loadres)
                textures.push_back(loadres.value());
            
            
        }
        else
        {
            //load from filesystem
            auto loadRes = Image::LoadFromFileDataFormat(str.C_Str());
            if (!loadRes)
                continue;
            auto image = CreateRef<Image>(std::move(loadRes.value()));
            auto textureAsset = CreateRef<TextureAsset>(image, typeName);
            textures.push_back(textureAsset);
        }
        
    }
    return textures;
}

std::optional<Ref<TextureAsset>> ModelAsset::LoadEmbeddedTexture(size_t index, aiTexture** textures, const std::string& typeName)
{
    auto iter = m_EmbeddedTextureCache.find(index);
    if (iter != m_EmbeddedTextureCache.end())
    {
        return iter->second;
    }
    aiTexture* texture = textures[index];
    if (texture->mHeight == 0)
    {

        auto compressedImageLoadres = Image::LoadFromMemDataFormat((unsigned char*)texture->pcData, texture->mWidth);
        if (!compressedImageLoadres)
        {
            Log::Error("Failed to load embedded compressed image {0}:{1}", __FILE__, __LINE__);
            return std::nullopt;
        }
        auto image = CreateRef<Image>(std::move(compressedImageLoadres.value()));
        auto textureAsset = CreateRef<TextureAsset>(image, typeName);
        return textureAsset;

    }
    else
    {
        auto loadRes = Image::LoadFromMemDataRGBA8888(
            reinterpret_cast<const char*>(texture->pcData),
            texture->mWidth,
            texture->mHeight);
        if (!loadRes)
        {
            Log::Error("Failed to load embedded RGBA8888 image {0}:{1}", __FILE__, __LINE__);
            return std::nullopt;
        }

        auto image = CreateRef<Image>(std::move(loadRes.value()));
        auto textureAsset = CreateRef<TextureAsset>(image, typeName);
        return textureAsset;
    }
}

std::optional<Ref<TextureAsset>> ModelAsset::LoadFileTexture(const std::string& path, const std::string& typeName)
{
    return std::optional<Ref<TextureAsset>>();
}


AETHER_NAMESPACE_END