#pragma once
#include "Render/RenderApi.h"
#include "Core/Math.h"
namespace Aether::Text
{
struct Quad
{
    Vec2f position=Vec2f(0.0,0.0);// screen space, left-top corner
    Vec2f size=Vec2f(100,100);// screen space
    Vec2f uv0=Vec2f(0.0,0.0);
    Vec2f uv1=Vec2f(1.0,1.0);
    uint32_t glyphIndex=0;// glyph index in device buffer
    float z=0.0f;// z value in screen space
};

} // namespace Aether