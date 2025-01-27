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
    inline const Vec2f& GetPosition() const
    {
        return m_Position;
    }
    inline const Vec2f& GetSize() const
    {
        return m_Size;
    }
    inline const Vec4f& GetColor() const
    {
        return m_Color;
    }
    inline const Vec2f& GetUVOffset() const
    {
        return m_UVOffset;
    }
    inline const Vec2f& GetUVSize() const
    {
        return m_UVSize;
    }
    inline const Ref<DeviceTexture>& GetTexture() const
    {
        return m_Texture;
    }
    inline const Ref<DeviceShader>& GetShader() const
    {
        return m_Shader;
    }
private:
    Ref<DeviceShader> m_Shader;
    Ref<DeviceTexture> m_Texture;
    Vec2f m_Position; // screen space, left-top corner
    Vec2f m_Size;     // screen space
    Vec4f m_Color;    // [0,1]
    Vec2f m_UVOffset; // in [0,1] texture space ,offset in left-bottom corner
    Vec2f m_UVSize;   // in [0,1] texture space
   
};
} // namespace UI
} // namespace Aether