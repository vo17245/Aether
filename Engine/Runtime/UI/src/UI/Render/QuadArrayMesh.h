#pragma once
#include "Render/Mesh/Mesh.h"
#include "Quad.h"
#include "Render/Mesh/VertexBufferLayout.h"
namespace Aether::UI
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
        // uv buffer
        m_UvBufferIndex=m_Mesh.buffers.size();
        m_Mesh.buffers.push_back({});
        // color buffer
        m_ColorBufferIndex=m_Mesh.buffers.size();
        m_Mesh.buffers.push_back({});
        // index buffer
        m_IndexBufferIndex=m_Mesh.buffers.size();
        m_Mesh.buffers.push_back({});
        // position buffer view
        m_PositionBufferViewIndex=m_Mesh.bufferViews.size();
        m_Mesh.bufferViews.push_back({0,0,m_PositionBufferIndex,Mesh::Target::Vertex});
        // uv buffer view
        m_UvBufferViewIndex=m_Mesh.bufferViews.size();
        m_Mesh.bufferViews.push_back({0,0,m_UvBufferIndex,Mesh::Target::Vertex});
        // color buffer view
        m_ColorBufferViewIndex=m_Mesh.bufferViews.size();
        m_Mesh.bufferViews.push_back({0,0,m_ColorBufferIndex,Mesh::Target::Vertex});
        // index buffer view
        m_IndexBufferViewIndex=m_Mesh.bufferViews.size();
        m_Mesh.bufferViews.push_back({0,0,m_IndexBufferIndex,Mesh::Target::Index});
        // position accessor
        m_PositionAccessorIndex=m_Mesh.accessors.size();
        m_Mesh.accessors.push_back({m_PositionBufferViewIndex,
        0,
        Mesh::ComponentType::FLOAT32,
        0,
        Mesh::Type::VEC3});
        // uv accessor
        m_UvAccessorIndex=m_Mesh.accessors.size();
        m_Mesh.accessors.push_back({m_UvBufferViewIndex,
        0,
        Mesh::ComponentType::FLOAT32,
        0,
        Mesh::Type::VEC2});
        // color accessor
        m_ColorAccessorIndex=m_Mesh.accessors.size();
        m_Mesh.accessors.push_back({m_ColorBufferViewIndex,
        0,
        Mesh::ComponentType::FLOAT32,
        0,
        Mesh::Type::VEC4});
        // index accessor
        m_IndexAccessorIndex=m_Mesh.accessors.size();
        m_Mesh.accessors.push_back({m_IndexBufferViewIndex,
        0,
        Mesh::ComponentType::UINT32,
        0,
        Mesh::Type::SCALAR});
        // primitive
        m_Mesh.primitive.attributes[Mesh::Attribute::POSITION]=m_PositionAccessorIndex;
        m_Mesh.primitive.attributes[Mesh::Attribute::TEXCOORD]=m_UvAccessorIndex;
        m_Mesh.primitive.attributes[Mesh::Attribute::COLOR]=m_ColorAccessorIndex;
        m_Mesh.primitive.index=m_IndexAccessorIndex;
    }
    void PushQuad(const Quad& quad)
    {
        const auto& position=quad.GetPosition();
        const auto& size=quad.GetSize();
        const auto& uvOffset=quad.GetUVOffset();
        const auto& uvSize=quad.GetUVSize();
        const auto& color=quad.GetColor();

        std::array<Vec3f,4> positions=
        {
            position,//left-top
            position+Vec3f(size.x(),0,0),//right-top
            position+Vec3f(size.x(),size.y(),0),//right-bottom
            position+Vec3f(0,size.y(),0)//left-bottom
        };
        std::array<Vec2f,4> uvs=
        {
            uvOffset,//left-top
            uvOffset+Vec2f(uvSize.x(),0),//right-top
            uvOffset+uvSize,//right-bottom
            uvOffset+Vec2f(0,uvSize.y())//left-bottom
        };
        std::array<Vec4f,4> colors=
        {
            color,
            color,
            color,
            color
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
        // append uv buffer
        auto& uvBuffer=m_Mesh.buffers[m_UvBufferIndex];
        uvBuffer.insert(uvBuffer.end(),(uint8_t*)uvs.data(),(uint8_t*)uvs.data()+sizeof(uvs));
        // append color buffer
        auto& colorBuffer=m_Mesh.buffers[m_ColorBufferIndex];
        colorBuffer.insert(colorBuffer.end(),(uint8_t*)colors.data(),(uint8_t*)colors.data()+sizeof(colors));
        // append index buffer
        auto& indexBuffer=m_Mesh.buffers[m_IndexBufferIndex];
        indexBuffer.insert(indexBuffer.end(),(uint8_t*)indices.data(),(uint8_t*)indices.data()+sizeof(indices));
        // update position buffer view
        auto& positionBufferView=m_Mesh.bufferViews[m_PositionBufferViewIndex];
        positionBufferView.size=positionBuffer.size();
        // update uv buffer view
        auto& uvBufferView=m_Mesh.bufferViews[m_UvBufferViewIndex];
        uvBufferView.size=uvBuffer.size();
        // update color buffer view
        auto& colorBufferView=m_Mesh.bufferViews[m_ColorBufferViewIndex];
        colorBufferView.size=colorBuffer.size();
        // update index buffer view
        auto& indexBufferView=m_Mesh.bufferViews[m_IndexBufferViewIndex];
        indexBufferView.size=indexBuffer.size();
        // update position accessor
        auto& positionAccessor=m_Mesh.accessors[m_PositionAccessorIndex];
        positionAccessor.count+=positions.size();
        // update uv accessor
        auto& uvAccessor=m_Mesh.accessors[m_UvAccessorIndex];
        uvAccessor.count+=uvs.size();
        // update color accessor
        auto& colorAccessor=m_Mesh.accessors[m_ColorAccessorIndex];
        colorAccessor.count+=colors.size();
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
    uint32_t m_UvBufferIndex=0;
    uint32_t m_ColorBufferIndex=0;
    uint32_t m_IndexBufferIndex=0;
    uint32_t m_PositionBufferViewIndex=0;
    uint32_t m_UvBufferViewIndex=0;
    uint32_t m_ColorBufferViewIndex=0;
    uint32_t m_IndexBufferViewIndex=0;
    uint32_t m_PositionAccessorIndex=0;
    uint32_t m_UvAccessorIndex=0;
    uint32_t m_ColorAccessorIndex=0;
    uint32_t m_IndexAccessorIndex=0;
    
};
}