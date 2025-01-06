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
};
} // namespace Kamui