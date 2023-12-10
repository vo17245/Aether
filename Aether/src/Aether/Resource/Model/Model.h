#pragma once
#include "Buffer.h"
#include "Node.h"
#include "Accessor.h"
#include "Aether/Resource/Image.h"
#include "Material.h"
#include "Primitive.h"
#include "Mesh.h"

namespace Aether
{
    struct Model
    {
        std::vector<Buffer> buffers;
        std::vector<BufferView> bufferViews;
        std::vector<Accessor> accessors;
        std::vector<Primitive> primitives;
        std::vector<Mesh> meshes;
        std::vector<Image> images;
        std::vector<Material> materials;
        std::vector<Node> nodes;
        // check if a cycle in nodes 
        bool HasCycle();
    };
}