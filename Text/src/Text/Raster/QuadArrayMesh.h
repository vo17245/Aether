#pragma once
#include "Render/Mesh/Mesh.h"
#include "Render/Mesh/VertexBufferLayout.h"
#include "Quad.h"
namespace Aether::Text
{
class QuadArrayMesh
{
public:
    QuadArrayMesh()
    {
        // init
        // position buffer
        m_PositionBufferIndex=m_Mesh.buffers.size();
        m_Mesh.buffers.push_back({});        
        
        // index buffer
        m_IndexBufferIndex=m_Mesh.buffers.size();
        m_Mesh.buffers.push_back({});
        // position buffer view
        m_PositionBufferViewIndex=m_Mesh.bufferViews.size();
        m_Mesh.bufferViews.push_back({0,0,m_PositionBufferIndex,Mesh::Target::Vertex});
       
        // index buffer view
        m_IndexBufferViewIndex=m_Mesh.bufferViews.size();
        m_Mesh.bufferViews.push_back({0,0,m_IndexBufferIndex,Mesh::Target::Index});
        // position accessor
        m_PositionAccessorIndex=m_Mesh.accessors.size();
        m_Mesh.accessors.push_back({m_PositionBufferViewIndex,
        0,
        Mesh::ComponentType::FLOAT32,
        0,
        Mesh::Type::VEC3,
        0});
        // index accessor
        m_IndexAccessorIndex=m_Mesh.accessors.size();
        m_Mesh.accessors.push_back({m_IndexBufferViewIndex,
        0,
        Mesh::ComponentType::UINT32,
        0,
        Mesh::Type::SCALAR,
        0});
        // primitive
        m_Mesh.primitive.attributes[Mesh::Primitive::Attribute::POSITION]=m_PositionAccessorIndex;
        m_Mesh.primitive.index=m_IndexAccessorIndex;
    }
    void PushQuad(const Quad& quad)
    {
        const auto& position=quad.position;
        const auto& size=quad.size;
        const auto& glyphIndex=quad.glyphIndex;

        std::array<Vec3f,4> positions=
        {
            position,//left-top
            position+Vec3f(size.x(),0,0),//right-top
            position+Vec3f(size.x(),size.y(),0),//right-bottom
            position+Vec3f(0,size.y(),0)//left-bottom
        };
        
        
        uint32_t lastVertexCount=m_Mesh.accessors[m_PositionAccessorIndex].count;
        std::array<uint32_t,6> indices=
        {
            0+lastVertexCount,1+lastVertexCount,2+lastVertexCount,
            0+lastVertexCount,2+lastVertexCount,3+lastVertexCount
        };
        // append position buffer
        auto& positionBuffer=m_Mesh.buffers[m_PositionBufferIndex];
        positionBuffer.insert(positionBuffer.end(),(uint8_t*)positions.data(),(uint8_t*)positions.data()+sizeof(positions));
        
        // append index buffer
        auto& indexBuffer=m_Mesh.buffers[m_IndexBufferIndex];
        indexBuffer.insert(indexBuffer.end(),(uint8_t*)indices.data(),(uint8_t*)indices.data()+sizeof(indices));
        // update position buffer view
        auto& positionBufferView=m_Mesh.bufferViews[m_PositionBufferViewIndex];
        positionBufferView.size=positionBuffer.size();
       
      
        // update index buffer view
        auto& indexBufferView=m_Mesh.bufferViews[m_IndexBufferViewIndex];
        indexBufferView.size=indexBuffer.size();
        // update position accessor
        auto& positionAccessor=m_Mesh.accessors[m_PositionAccessorIndex];
        positionAccessor.count+=positions.size();
        
        // update index accessor
        auto& indexAccessor=m_Mesh.accessors[m_IndexAccessorIndex];
        indexAccessor.count+=indices.size();
    }
    inline const Mesh& GetMesh() const
    {
        return m_Mesh;
    }
    inline Mesh& GetMesh()
    {
        return m_Mesh;
    }

private:
    Mesh m_Mesh;
    uint32_t m_PositionBufferIndex=0;
    uint32_t m_IndexBufferIndex=0;
    uint32_t m_PositionBufferViewIndex=0;
    uint32_t m_IndexBufferViewIndex=0;
    uint32_t m_PositionAccessorIndex=0;
    uint32_t m_IndexAccessorIndex=0;
};
}