#include "SceneViewPanel.h"
#include "Aether/Resource/Model/Accessor.h"
#include "Aether/Resource/Model/BufferView.h"
#include <cstddef>
#include <stdint.h>
namespace Aether
{
    //create a quad in [-1,1] with 2d texture uv
    static Ref<Model> CreateScreenQuad()
    {
        Ref<Model> model=CreateRef<Model>();
        //1.buffer
        //1.1 vertex buffer
        float vertices[16]=
        {
            -1,-1,0,0,//left buttom
            1,-1,1,0,//right buttom
            1,1,1,1,//right top
            -1,1,0,1,//left top

        };
        constexpr size_t vertexBufferIndex=0;
        model->buffers.emplace_back((std::byte*)vertices,
        ((std::byte*)vertices)+sizeof(float)*16);
        //1.2 index buffer
        uint32_t indices[6]=
        {
            0,1,2,
            2,3,0
        };
        constexpr size_t indexBufferIndex=1;
        model->buffers.emplace_back((std::byte*)indices,
        ((std::byte*)indices)+sizeof(uint32_t)*6);
        //2.buffer view
        constexpr size_t vertexBufferViewIndex=0;
        model->bufferViews.emplace_back(vertexBufferIndex,0,
        model->buffers[vertexBufferIndex].size(),BufferView::Target::VERTEX_BUFFER,
        model.get());
        const size_t indexBufferViewIndex=1;
        model->bufferViews.emplace_back(indexBufferIndex,0,
        model->buffers[indexBufferIndex].size(),BufferView::Target::INDEX_BUFFER,
        model.get());
        //3.accessor
        //3.1 position
        size_t positionAccessorIndex=model->accessors.size();
        model->accessors.emplace_back(vertexBufferIndex, 
        4*sizeof(float),ElementType::VEC2,0,model.get());
        //3.2 texture uv
        size_t uvAccessorIndex=model->accessors.size();
        model->accessors.emplace_back(vertexBufferIndex, 
        4*sizeof(float),ElementType::VEC2,2*sizeof(float),model.get());
        //3.3 vertex index
        size_t indexAccessorIndex=model->accessors.size();
        model->accessors.emplace_back(indexBufferIndex,
        sizeof(uint32_t),ElementType::UNSIGNED_INT32,0,model.get());
        //4.primitive
        size_t primitiveIndex=model->primitives.size();
        model->primitives.emplace_back(model.get());
        model->primitives[primitiveIndex].GetAttributes()["POSITION"]=
        positionAccessorIndex;
        model->primitives[primitiveIndex].GetAttributes()["TEXCOORD_0"]=uvAccessorIndex;
        model->primitives[primitiveIndex].SetIndices(indexAccessorIndex);
        //5.mesh
        model->meshes.empl
        //6.node
    }
}