#pragma once
#include "Aether/Core/Math.h"
#include <vector>

namespace Aether
{
    struct Node
    {
        Vec3 translation;
        Vec3 rotation;
        Vec3 scaling;
        size_t mesh;
        std::vector<size_t> children;
        
    };
}