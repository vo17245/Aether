#pragma once
#include <Scene/Scene.h>
namespace Aether::UI
{
    struct Node
    {
        EntityId id;
        std::vector<Node*> children;
    };
}