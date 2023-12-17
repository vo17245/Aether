#pragma once
#include "Aether/Core/Math.h"
#include <vector>

namespace Aether
{
    struct Node
    {
        Node(const Node&)=default;
        Node(Node&&)=default;
        Node()=default;
        Vec3 translation=Vec3(0,0,0);
        Vec3 scaling=Vec3(1,1,1);
        //rotation following right-hand screw rule
        Vec3 rotationAxis=Vec3(1,1,1);
        Real rotationAngle=0;
        std::vector<size_t> meshes; 
        std::vector<size_t> children;
    };
}