#pragma once
#include "Primitive.h"
#include <stdint.h>
namespace Aether
{
    class Mesh
    {
    public:
        Mesh() = default;
        Mesh(Mesh&&) = default;
        Mesh& operator=(Mesh&&) = default;
        Mesh(const Mesh&) = delete;
        Mesh& operator=(const Mesh&) = delete;
        ~Mesh() = default;
        size_t primitive;
        size_t material;
    };
}