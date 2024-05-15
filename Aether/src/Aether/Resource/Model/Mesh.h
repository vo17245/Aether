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
        Mesh(size_t _primitive, size_t _material)
            : primitive(_primitive), material(_material)
        {}
        size_t primitive;
        size_t material;
    };
}