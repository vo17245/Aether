#pragma once
#include "Render/RenderApi.h"
#include "Core/Math.h"
namespace Aether
{
namespace UI
{
struct QuadDesc
{
    Vec2f position;
    Vec2f size;
    Vec4f color;
    Vec2f uvOffset;
};
class Quad
{
public:
    Quad(const QuadDesc& desc) :
        m_Position(desc.position), m_Size(desc.size), m_Color(desc.color), m_UVOffset(desc.uvOffset)
    {
    }
    void SetTexture(const Ref<DeviceTexture>& texture)
    {
        m_Texture=texture;
    }
    void SetShader(const Ref<DeviceShader>& shader)
    {
        m_Shader=shader;
    }
private:
    DeviceMesh m_Mesh;
    Ref<DeviceShader> m_Shader;
    Ref<DeviceTexture> m_Texture;
    Vec2f m_Position; // screen space, left-top corner
    Vec2f m_Size;     // screen space
    Vec4f m_Color;    // [0,1]
    Vec2f m_UVOffset; // in [0,1] texture space ,offset in left-bottom corner
    Vec2f m_UVSize;   // in [0,1] texture space
    bool CreateMesh()
    {
        static std::array<Vec2f,4> positions=
        {
            m_Position,//left-top
            m_Position+Vec2f(m_Size.x(),0),//right-top
            m_Position+m_Size,//right-bottom
            m_Position+Vec2f(0,m_Size.y())//left-bottom
        };
        static std::array<Vec2f,4> uvs=
        {
            m_UVOffset,//left-top
            m_UVOffset+Vec2f(m_UVSize.x(),0),//right-top
            m_UVOffset+m_UVSize,//right-bottom
            m_UVOffset+Vec2f(0,m_UVSize.y())//left-bottom
        };
        static std::array<Vec4f,4> colors=
        {
            m_Color,
            m_Color,
            m_Color,
            m_Color
        };
        static std::array<uint32_t,6> indices=
        {
            0,1,2,
            0,2,3
        };
        Mesh mesh;
        // position buffer
        uint32_t positionBufferIndex=mesh.buffers.size();
        mesh.buffers.push_back(Mesh::Buffer((uint8_t*)positions.data(),(uint8_t*)positions.data()+positions.size()*sizeof(Vec2f)));
        // uv buffer
        uint32_t uvBufferIndex=mesh.buffers.size();
        mesh.buffers.push_back(Mesh::Buffer((uint8_t*)uvs.data(),(uint8_t*)uvs.data()+uvs.size()*sizeof(Vec2f)));
        // color buffer
        uint32_t colorBufferIndex=mesh.buffers.size();
        mesh.buffers.push_back(Mesh::Buffer((uint8_t*)colors.data(),(uint8_t*)colors.data()+colors.size()*sizeof(Vec4f)));
        // index buffer
        uint32_t indexBufferIndex=mesh.buffers.size();
        mesh.buffers.push_back(Mesh::Buffer((uint8_t*)indices.data(),(uint8_t*)indices.data()+indices.size()*sizeof(uint32_t)));
        // position buffer view
        uint32_t positionBufferViewIndex=mesh.bufferViews.size();
        mesh.bufferViews.push_back({
            0,
            positions.size()*sizeof(Vec2f),
            positionBufferIndex,
            Mesh::Target::Vertex
        });
        // uv buffer view
        uint32_t uvBufferViewIndex=mesh.bufferViews.size();
        mesh.bufferViews.push_back({
            0,
            uvs.size()*sizeof(Vec2f),
            uvBufferIndex,
            Mesh::Target::Vertex
        });
        // color buffer view
        uint32_t colorBufferViewIndex=mesh.bufferViews.size();
        mesh.bufferViews.push_back({
            0,
            colors.size()*sizeof(Vec4f),
            colorBufferIndex,
            Mesh::Target::Vertex
        });
        // index buffer view
        uint32_t indexBufferViewIndex=mesh.bufferViews.size();
        mesh.bufferViews.push_back({
            0,
            indices.size()*sizeof(uint32_t),
            indexBufferIndex,
            Mesh::Target::Index
        });
        // position accessor
        uint32_t positionAccessorIndex=mesh.accessors.size();
        mesh.accessors.push_back({
            positionBufferViewIndex,
            0,
            Mesh::ComponentType::FLOAT32,
            positions.size(),
            Mesh::Type::VEC2,
            0
        });
        // uv accessor
        uint32_t uvAccessorIndex=mesh.accessors.size();
        mesh.accessors.push_back({
            uvBufferViewIndex,
            0,
            Mesh::ComponentType::FLOAT32,
            uvs.size(),
            Mesh::Type::VEC2,
            0
        });
        // color accessor
        uint32_t colorAccessorIndex=mesh.accessors.size();
        mesh.accessors.push_back({
            colorBufferViewIndex,
            0,
            Mesh::ComponentType::FLOAT32,
            colors.size(),
            Mesh::Type::VEC4,
            0
        });
        // index accessor
        uint32_t indexAccessorIndex=mesh.accessors.size();
        mesh.accessors.push_back({
            indexBufferViewIndex,
            0,
            Mesh::ComponentType::UINT32,
            indices.size(),
            Mesh::Type::SCALAR,
            0
        });
        // primitive
        mesh.primitive.attributes[Mesh::Primitive::Attribute::POSITION]=positionAccessorIndex;
        mesh.primitive.attributes[Mesh::Primitive::Attribute::TEXCOORD]=uvAccessorIndex;
        mesh.primitive.attributes[Mesh::Primitive::Attribute::COLOR]=colorAccessorIndex;
        mesh.primitive.index=indexAccessorIndex;
        auto deviceMesh=CreateDeviceMesh(mesh);
        if(!deviceMesh)
        {
            return false;
        }
        m_Mesh=std::move(deviceMesh.value());
        return true;
    }
};
} // namespace UI
} // namespace Aether