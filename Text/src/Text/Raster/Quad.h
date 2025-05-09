#pragma once
#include "Render/RenderApi.h"
#include "Core/Math.h"
namespace Aether::Text
{
struct Quad
{
    Vec2f position=Vec2f(0.0,0.0);// screen space, left-top corner
    Vec2f size=Vec2f(100,100);// screen space
    uint32_t glyphIndex=0;// glyph index in device buffer
};

} // namespace Aether