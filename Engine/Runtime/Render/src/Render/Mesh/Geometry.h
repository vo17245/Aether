#pragma once
#include "Mesh.h"
#include "Core/Math.h"
#include <cstdint>
namespace Aether {
class Geometry
{
public:
    static Mesh CreateBox();
    static Mesh CreateSphere(float radius = 1, uint32_t segments = 32);
    /**
     * @return 
     * vertex{vec2f position;vec2f texCoord;}
     * pisition [-1,1]
     * texCoord [0,1]
    */
    static Mesh CreateQuad();
};
} // namespace Kamui