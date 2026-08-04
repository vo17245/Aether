#pragma once
#include "Core/Math.h"
#include <Render/RHI.h>

namespace Aether::UI
{
struct QuadDesc
{
    Vec3f position = Vec3f(0.0, 0.0, 0.0);
    Vec2f size = Vec2f(100, 100);
    Vec4f color = Vec4f(1.0, 1.0, 1.0, 1.0);
    Vec2f uvOffset = Vec2f(0.0, 0.0);
    Vec2f uvSize = Vec2f(1.0, 1.0);
};
class Quad
{
public:
    Quad(const QuadDesc& desc) :
        m_Position(desc.position), m_Size(desc.size), m_Color(desc.color), m_UVOffset(desc.uvOffset), m_UVSize(desc.uvSize)
    {
    }
    void SetTexture(const Ref<rhi::Texture2D>& texture)
    {
        m_Texture = texture;
    }
    void SetShader(const Ref<rhi::PixelShader>& shader)
    {
        m_Shader = shader;
    }
    inline const Vec3f& GetPosition() const { return m_Position; }
    inline void SetPosition(const Vec3f& position) { m_Position = position; }
    inline const Vec2f& GetSize() const { return m_Size; }
    inline void SetSize(const Vec2f& size) { m_Size = size; }
    inline const Vec4f& GetColor() const { return m_Color; }
    inline const Vec2f& GetUVOffset() const { return m_UVOffset; }
    inline const Vec2f& GetUVSize() const { return m_UVSize; }
    inline const Ref<rhi::Texture2D>& GetTexture() const { return m_Texture; }
    inline const Ref<rhi::PixelShader>& GetShader() const { return m_Shader; }
    inline void SetColor(const Vec4f& color) { m_Color = color; }

private:
    Ref<rhi::PixelShader> m_Shader;
    Ref<rhi::Texture2D> m_Texture;
    Vec3f m_Position;
    Vec2f m_Size;
    Vec4f m_Color;
    Vec2f m_UVOffset;
    Vec2f m_UVSize;
};
} // namespace Aether::UI
