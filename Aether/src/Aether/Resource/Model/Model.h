#pragma once
#include "Buffer.h"
#include "Node.h"
#include "Accessor.h"
#include "Aether/Resource/Image.h"
#include "Material.h"
#include "Primitive.h"
#include "Mesh.h"
#include "BufferView.h"
namespace Aether
{
    class Model
    {
    public:
        Model()=default;
        Model(Model&&) = default;
        Model& operator=(Model&&) = default;
        Model(const Model&) = delete;
        Model& operator=(const Model&) = delete;
        ~Model() { Unbind(); }
        std::vector<Buffer> buffers;
        std::vector<BufferView> bufferViews;
        std::vector<Accessor> accessors;
        std::vector<Primitive> primitives;
        std::vector<Mesh> meshes;
        std::vector<Node> nodes;
        bool hasBind=false;
        // check if a cycle in nodes 
        bool HasCycle();
        void Bind();//make instance in gpu
        void Unbind();//release instance in gpu
        void Render();//drawelement for each primitive
    };
}