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
        // glyph index buffer
        m_GlyphIndexBufferIndex=m_Mesh.buffers.size();
        m_Mesh.buffers.push_back({});
        // index buffer
        m_IndexBufferIndex=m_Mesh.buffers.size();
        m_Mesh.buffers.push_back({});
        // uv buffer
        m_UvBufferIndex=m_Mesh.buffers.size();
        m_Mesh.buffers.push_back({});
        // position buffer view
        m_PositionBufferViewIndex=m_Mesh.bufferViews.size();
        m_Mesh.bufferViews.push_back({0,0,m_PositionBufferIndex,Mesh::Target::Vertex});
        // glyph index buffer view
        m_GlyphIndexBufferViewIndex=m_Mesh.bufferViews.size();
        m_Mesh.bufferViews.push_back({0,0,m_GlyphIndexBufferIndex,
        Mesh::Target::Vertex});
        // index buffer view
        m_IndexBufferViewIndex=m_Mesh.bufferViews.size();
        m_Mesh.bufferViews.push_back({0,0,m_IndexBufferIndex,Mesh::Target::Index});
        // uv buffer view
        m_UvBufferViewIndex=m_Mesh.bufferViews.size();
        m_Mesh.bufferViews.push_back({0,0,m_UvBufferIndex,Mesh::Target::Vertex});
        // position accessor
        m_PositionAccessorIndex=m_Mesh.accessors.size();
        m_Mesh.accessors.push_back({m_PositionBufferViewIndex,
        0,
        Mesh::ComponentType::FLOAT32,
        0,
        Mesh::Type::VEC3,
        0});
        // glyph index accessor
        m_GlyphIndexAccessorIndex=m_Mesh.accessors.size();
        m_Mesh.accessors.push_back({m_GlyphIndexBufferViewIndex,
        0,
        Mesh::ComponentType::UINT32,
        0,
        Mesh::Type::SCALAR,
        0});
        // index accessor
        m_IndexAccessorIndex=m_Mesh.accessors.size();
        m_Mesh.accessors.push_back({m_IndexBufferViewIndex,
        0,
        Mesh::ComponentType::UINT32,
        0,
        Mesh::Type::SCALAR,
        0});
        // uv accessor
        m_UvAccessorIndex=m_Mesh.accessors.size();
        m_Mesh.accessors.push_back({m_UvBufferViewIndex,
        0,
        Mesh::ComponentType::FLOAT32,
        0,
        Mesh::Type::VEC2,
        0});
        // primitive
        m_Mesh.primitive.attributes[Mesh::Primitive::Attribute::POSITION]=m_PositionAccessorIndex;
        m_Mesh.primitive.attributes[Mesh::Primitive::Attribute::NORMAL]=m_GlyphIndexAccessorIndex;
        m_Mesh.primitive.attributes[Mesh::Primitive::Attribute::TEXCOORD]=m_UvAccessorIndex;
        m_Mesh.primitive.index=m_IndexAccessorIndex;
    }
    void PushQuad(const Quad& quad)
    {
        Vec3f  position(quad.position.x(),quad.position.y(),0);
        const auto& size=quad.size;
        const auto& glyphIndex=quad.glyphIndex;

        std::array<Vec3f,4> positions=
        {
            position,//left-top
            position+Vec3f(size.x(),0,quad.z),//right-top
            position+Vec3f(size.x(),size.y(),quad.z),//right-bottom
            position+Vec3f(0,size.y(),quad.z)//left-bottom
        };
        std::array<Vec2f,4> uvs=
        {
            Vec2f(quad.uv0.x(),quad.uv1.y()),
            Vec2f(quad.uv1.x(),quad.uv1.y()),
            Vec2f(quad.uv1.x(),quad.uv0.y()),
            Vec2f(quad.uv0.x(),quad.uv0.y())
        };
        
        
        uint32_t lastVertexCount=m_Mesh.accessors[m_PositionAccessorIndex].count;
        std::array<uint32_t,6> indices=
        {
            0+lastVertexCount,1+lastVertexCount,2+lastVertexCount,
            0+lastVertexCount,2+lastVertexCount,3+lastVertexCount
        };
        // append position buffer
        auto& positionBuffer=m_Mesh.buffers[m_PositionBufferIndex];
        positionBuffer.insert(positionBuffer.end(),(uint8_t*)positions.data(),
        (uint8_t*)positions.data()+sizeof(positions));
        // append glyph index buffer
        auto& glyphIndexBuffer=m_Mesh.buffers[m_GlyphIndexBufferIndex];
        for(size_t i=0;i<4;i++)
        {
glyphIndexBuffer.insert(glyphIndexBuffer.end(),
        (uint8_t*)&glyphIndex,(uint8_t*)&glyphIndex+sizeof(glyphIndex));
        }
        
        // append index buffer
        auto& indexBuffer=m_Mesh.buffers[m_IndexBufferIndex];
        indexBuffer.insert(indexBuffer.end(),(uint8_t*)indices.data(),(uint8_t*)indices.data()+sizeof(indices));
        // append uv buffer
        auto& uvBuffer=m_Mesh.buffers[m_UvBufferIndex];
        uvBuffer.insert(uvBuffer.end(),(uint8_t*)uvs.data(),
        (uint8_t*)uvs.data()+sizeof(uvs));
        // update position buffer view
        auto& positionBufferView=m_Mesh.bufferViews[m_PositionBufferViewIndex];
        positionBufferView.size=positionBuffer.size();
        // update glyph index buffer view
        auto& glyphIndexBufferView=m_Mesh.bufferViews[m_GlyphIndexBufferViewIndex];
        glyphIndexBufferView.size=glyphIndexBuffer.size();

      
        // update index buffer view
        auto& indexBufferView=m_Mesh.bufferViews[m_IndexBufferViewIndex];
        indexBufferView.size=indexBuffer.size();
        // update uv buffer view
        auto& uvBufferView=m_Mesh.bufferViews[m_UvBufferViewIndex];
        uvBufferView.size=uvBuffer.size();
        // update position accessor
        auto& positionAccessor=m_Mesh.accessors[m_PositionAccessorIndex];
        positionAccessor.count+=positions.size();
        //update glyph index accessor
        auto& glyphIndexAccessor=m_Mesh.accessors[m_GlyphIndexAccessorIndex];
        glyphIndexAccessor.count+=4;
        // update index accessor
        auto& indexAccessor=m_Mesh.accessors[m_IndexAccessorIndex];
        indexAccessor.count+=indices.size();
        // update uv accessor
        auto& uvAccessor=m_Mesh.accessors[m_UvAccessorIndex];
        uvAccessor.count+=uvs.size();
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
    // buffer index
    uint32_t m_PositionBufferIndex=0;
    uint32_t m_IndexBufferIndex=0;
    uint32_t m_GlyphIndexBufferIndex=0;
    uint32_t m_UvBufferIndex=0;
    // buffer view index
    uint32_t m_PositionBufferViewIndex=0;
    uint32_t m_IndexBufferViewIndex=0;
    uint32_t m_GlyphIndexBufferViewIndex=0;
    uint32_t m_UvBufferViewIndex=0;

    // accessor index
    uint32_t m_PositionAccessorIndex=0;
    uint32_t m_IndexAccessorIndex=0;
    uint32_t m_GlyphIndexAccessorIndex=0;
    uint32_t m_UvAccessorIndex=0;
};
}