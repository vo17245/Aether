#pragma once
#include <optional>
#include <string>
#include <unordered_map>
#include "Aether/Render/VertexArray.h"
#include "Aether/Render/VertexBuffer.h"
#include "Aether/Render/IndexBuffer.h"
namespace Aether
{
    struct Primitive
    {
        std::unordered_map<std::string, size_t> attributes;
        std::vector<uint32_t> indices;
        Scope<VertexArray> vao;
        Scope<VertexBuffer> vbo;
        Scope<IndexBuffer> ibo;
        //submit data to gpu
        void Bind();
        //release data in gpu
        void Unbind();
    };
}