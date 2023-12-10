#pragma once
#include "Primitive.h"
#include <stdint.h>
namespace Aether
{
    struct Mesh
    {
        size_t primitive;
        size_t material;
    };
}