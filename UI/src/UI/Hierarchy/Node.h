#pragma once
#include <Scene/Scene.h>
namespace Aether::UI
{
    struct Node
    {
        EntityId id;
        Node* parent=nullptr;
        std::vector<Node*> children;
        void PushChild(Node* child)
        {
            children.push_back(child);
            child->parent=this;
        }
    };
}