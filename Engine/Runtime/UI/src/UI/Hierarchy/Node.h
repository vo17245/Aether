#pragma once
#include <Scene/Scene.h>
#include <Core/Borrow.h>
namespace Aether::UI
{
    class Hierarchy;
    struct Node
    {
        Node(Borrow<Hierarchy> hierarchy) :
            hierarchy(hierarchy)
        {
        }
        EntityId id;
        Node* parent=nullptr;
        std::vector<Node*> children;
        Borrow<Hierarchy> hierarchy;
        void PushChild(Node* child)
        {
            children.push_back(child);
            child->parent=this;
        }
        /**
         * @brief remove this node from hierarchy
         * @note this node will be deleted after this call (node always allocated on heap)
        */ 
        void Remove();
        template<typename T>
        T& GetComponent();
        template<typename T>
        bool HasComponent();
        template<typename T,typename... Args>
        T& AddComponent(Args&&... args);
    };
}