#pragma once
#include "Render/RenderApi.h"
#include "Core/Core.h"
#include "Atlas.h"
namespace Aether::Sprite
{
struct QuadInstance
{
    Mat2x3f affine;          // vec2 transformed=affine*ve3(pos,1.0);
    Vec4f uvOffsetAndScale; // xy: offset, zw: scale
};
struct Quad
{
    Borrow<Atlas> atlas;
    Mat2x3f affine;
    uint32_t frameIndex;
    uint32_t zOrder;
    Vec4f CalculateUvOffsetAndScale() const
    {
        const auto& frame = atlas->info.frameRange[frameIndex];
        Vec2f uvOffset = Vec2f(frame.min.x(), frame.max.y()).array() / Vec2f(atlas->texture->GetWidth(), atlas->texture->GetHeight()).array();
        Vec2f uvScale = (Vec2f((frame.max - frame.min).cast<float>()).array() / Vec2f(atlas->texture->GetWidth(), atlas->texture->GetHeight()).array()).cast<float>();
        return Vec4f(uvOffset.x(), uvOffset.y(), uvScale.x(), uvScale.y());
    }
};
} // namespace Aether::Sprite