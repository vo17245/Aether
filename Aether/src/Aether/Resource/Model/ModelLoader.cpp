#include "ModelLoader.h"
#include "Aether/Core/Assert.h"
#include "Aether/Core/Core.h"
#include "Aether/Core/Log.h"
#include "Aether/Resource/Model/Accessor.h"
#include "Aether/Resource/Model/Buffer.h"
#include "assimp/mesh.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <optional>
#include <stdint.h>
#include <unordered_map>
#include <vcruntime.h>
namespace Aether
{
    struct ProcessAssimpModelContext
    {
        std::unordered_map<aiNode*, size_t> nodeMap;
        std::unordered_map<size_t, size_t> meshMap;
    };
    static bool ProcessMesh(aiScene& ai_scene,size_t ai_mesh,
    Model& model,size_t node,
    ProcessAssimpModelContext& context)
    {
        model.primitives.emplace_back(&model);
        aiMesh* ai_meshPtr=ai_scene.mMeshes[ai_mesh];
        //indices?
        if(ai_meshPtr->HasFaces())
        {
            //copy
            std::vector<uint32_t> indices;
            for (size_t i = 0; i < ai_meshPtr->mNumFaces; i++)
            {
                aiFace face = ai_meshPtr->mFaces[i];
                if(face.mNumIndices!=3)
                {
                    AETHER_ASSERT(false&&"not triangle");
                    AETHER_DEBUG_LOG("face not triangle");
                    return false;
                }
                for (unsigned int j = 0; j < face.mNumIndices; j++)
                {
                    indices.push_back(face.mIndices[j]);
                }
                    
            }
            //buffer
            model.buffers.emplace_back((std::byte*)indices.data(),((std::byte*)indices.data())+indices.size()*sizeof(uint32_t));
            //bufferViews;
            model.bufferViews.emplace_back(model.buffers.size()-1,0,model.buffers.back().size(),
                BufferView::Target::INDEX_BUFFER,
                &model);
            //accessor
            model.accessors.emplace_back(model.bufferViews.size()-1,
                4,//sizeof(unsigned int) 
                ElementType::UNSIGNED_INT32, 0,
                &model);
            //indices
            model.primitives.back().SetIndices(model.accessors.size()-1);
        }
        //position
        if(ai_scene.mMeshes[ai_mesh]->HasPositions())
        {
            //copy data to buffer
            model.buffers.emplace_back((std::byte*)ai_scene.mMeshes[ai_mesh]->mVertices,
            ((std::byte*)ai_scene.mMeshes[ai_mesh]->mVertices)+
            ai_scene.mMeshes[ai_mesh]->mNumVertices*3*sizeof(float));
            //bufferview
            model.bufferViews.emplace_back(model.buffers.size()-1,0,
                model.buffers.back().size(),
                BufferView::Target::VERTEX_BUFFER,
                &model);
            //accessor
            model.accessors.emplace_back(model.buffers.size()-1,0,ElementType::VEC3,0,&model);
            //attribute
            model.primitives.back().GetAttributes()["POSITION"]=model.accessors.size()-1;
        }
        //normal
        if(ai_meshPtr->HasNormals())
        {
            //copy data to buffer
            model.buffers.emplace_back((std::byte*)ai_meshPtr->mNormals,((std::byte*)ai_meshPtr->mNormals)+
            ai_meshPtr->mNumVertices*3*sizeof(float));
            //bufferView
            model.bufferViews.emplace_back(model.buffers.size()-1,
                0,model.buffers.back().size(),
                BufferView::Target::VERTEX_BUFFER,
                &model);
            //accessor
            model.accessors.emplace_back(model.bufferViews.size()-1,
                0,ElementType::VEC3,0,&model);
            //attribute
            model.primitives.back().GetAttributes()["NORMAL"]=model.accessors.size()-1;
        }
        return true;
    }
    static bool ProcessNode(aiScene& ai_scene,aiNode* ai_node,
    Model& model,ssize_t parent,
    ProcessAssimpModelContext& context)
    {
        auto iter=context.nodeMap.find(ai_node);
        size_t node;
        if(iter!=context.nodeMap.end())
        {
            //node exist in model
            node=iter->second;
        }
        else
        {
            //create a new node;
            model.nodes.emplace_back();
            node=model.nodes.size()-1;
            //process its meshes
            for (size_t i = 0; i < ai_node->mNumMeshes; i++)
            {
                
                bool ret=ProcessMesh(ai_scene,ai_node->mMeshes[i],model,node,context);
                if (!ret)
                {
                    AETHER_DEBUG_LOG_ERROR("failed to process mesh");
                    return false;
                }
            }
            //process its transform
            aiVector3t<Real> scaling;
            aiVector3t<Real> rotationAxis;
            Real rotationAngle;
            aiVector3t<Real> position;
            ai_node->mTransformation.Decompose( scaling, rotationAxis ,rotationAngle , position);
            model.nodes[node].translation.x()=scaling.x;
            model.nodes[node].translation.y()=scaling.y;
            model.nodes[node].translation.z()=scaling.z;
            model.nodes[node].rotationAxis.x()=rotationAxis.x;
            model.nodes[node].rotationAxis.y()=rotationAxis.y;
            model.nodes[node].rotationAxis.z()=rotationAxis.z;
            model.nodes[node].scaling.x()=scaling.x;
            model.nodes[node].scaling.y()=scaling.y;
            model.nodes[node].scaling.z()=scaling.z;
            model.nodes[node].rotationAngle=rotationAngle;
            //process its children
            for(size_t i=0;i<ai_node->mNumChildren;i++)
            {
                bool ret=ProcessNode(ai_scene, ai_node->mChildren[i], 
                model,node,context);
                if(!ret)
                {
                    AETHER_DEBUG_LOG_ERROR("failed to process children node");
                    return false;
                }
            }
            //set context
            context.nodeMap[ai_node]=node;
        }
        //attach this node to parent
        if(parent>0)
        {
            model.nodes[parent].children.emplace_back(node);
        }
        return true;

    }
    std::optional<Ref<Model>> ModelLoader::LoadFromFile(const std::filesystem::path& path)
    {
        Ref<Model> res=CreateRef<Model>();
        Assimp::Importer import;
        aiScene* ai_scene = const_cast<aiScene*>(
            import.ReadFile(path.string(), aiProcess_Triangulate | aiProcess_FlipUVs)
        );
        if ((!ai_scene || ai_scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !ai_scene->mRootNode))
        {
            AETHER_DEBUG_LOG_ERROR("Failed to load model,model file path {0}",path.string());
            return std::nullopt;
        }
        ProcessAssimpModelContext context;
        bool ret=ProcessNode(*ai_scene, ai_scene->mRootNode, 
        *res, -1,context);
        if(!ret)
        {
            return std::nullopt;
        }
        return res;
    }
}