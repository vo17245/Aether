#pragma once
#include "Render/RenderApi.h"
#include "Core/Math.h"
namespace Aether
{
namespace UI
{
struct QuadDesc
{
    Vec3f position=Vec3f(0.0,0.0,0.0);// screen space, left-top corner
    Vec2f size=Vec2f(100,100);// screen space
    Vec4f color=Vec4f(1.0,1.0,1.0,1.0);// [0,1]
    Vec2f uvOffset=Vec2f(0.0,0.0);// in [0,1] texture space ,offset in left-bottom corner
    Vec2f uvSize=Vec2f(1.0,1.0);// in [0,1] texture space
};
class Quad
{
public:
    Quad(const QuadDesc& desc) :
        m_Position(desc.position), m_Size(desc.size), m_Color(desc.color), m_UVOffset(desc.uvOffset), m_UVSize(desc.uvSize)
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
    inline const Vec3f& GetPosition() const
    {
        return m_Position;
    }
    inline void SetPosition(const Vec3f& position)
    {
        m_Position=position;
    }
    inline const Vec2f& GetSize() const
    {
        return m_Size;
    }
    inline void SetSize(const Vec2f& size)
    {
        m_Size=size;
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
    inline void SetColor(const Vec4f& color)
    {
        m_Color=color;
    }
private:
    Ref<DeviceShader> m_Shader;
    Ref<DeviceTexture> m_Texture;
    Vec3f m_Position; // screen space, left-top corner
    Vec2f m_Size;     // screen space
    Vec4f m_Color;    // [0,1]
    Vec2f m_UVOffset; // in [0,1] texture space ,offset in left-bottom corner
    Vec2f m_UVSize;   // in [0,1] texture space
   
};
} // namespace UI
} // namespace Aether