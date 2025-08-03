#pragma once
#include "Render/RenderApi.h"
#include "Core/Core.h"
#include "Atlas.h"
namespace Aether::Sprite
{
struct alignas(16) Mat2x3
{
    float m00, m01, m02, _pad0; // 第一列
    float m10, m11, m12, _pad1; // 第二列
}; // 总共 32 字节
struct QuadInstance
{
    Mat2x3 affine;          // vec2 transformed=affine*ve3(pos,1.0);
    Vec4f uvOffsetAndScale; // xy: offset, zw: scale
};
struct Quad
{
    Borrow<Atlas> atlas;
    Mat2x3 affine;
    uint32_t frameIndex;
    uint32_t zOrder;
    Vec4f CalculateUvOffsetAndScale() const
    {
        const auto& frame = atlas->info.frameRange[frameIndex];
        Vec2f uvOffset = Vec2f(frame.min.x(), frame.max.y()).array() / Vec2f(atlas->texture->GetWidth(), atlas->texture->GetHeight()).array();
        Vec2f uvScale = Vec2f(frame.max - frame.min).array() / Vec2f(atlas->texture->GetWidth(), atlas->texture->GetHeight()).array();
        return Vec4f(uvOffset.x(), uvOffset.y(), uvScale.x(), uvScale.y());
    }
};
} // namespace Aether::Sprite