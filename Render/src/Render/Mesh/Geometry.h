#pragma once
#include "Grid.h"
#include "Core/Math.h"
#include <cstdint>
namespace Aether {
class Geometry
{
public:
    static Grid CreateBox();
    static Grid CreateSphere(float radius = 1, uint32_t segments = 32);
};
} // namespace Kamui