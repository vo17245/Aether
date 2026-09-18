#pragma once

#include "Physics/Types.h"
namespace Aether::Physics
{
struct CompoundChild
{
    Shape shape;
    LocalTransform transform;
    std::uint32_t userData = 0;
};

} // namespace Aether::Physics
