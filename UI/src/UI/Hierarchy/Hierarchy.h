#pragma once
#include <Scene/Scene.h>
#include <vector>
#include "Node.h"
#include "Component/Node.h"
#include "System/System.h"
#include <vector>
namespace Aether::UI
{
    class Hierarchy
    {
    public:
        Node* CreateNode()
        {
            Node* node = new Node();
            node->id = m_Scene.CreateEntity();
            m_Scene.AddComponent<NodeComponent>(node->id, node);
            return node;
        }
        void DestroyNode(Node* node)
        {
            m_Scene.DestroyEntity(node->id);
            for(auto child : node->children)
            {
                DestroyNode(child);
            }
            delete node;
        }
        Node* GetRoot()
        {
            return m_Root;
        }
        Scene& GetScene()
        {
            return m_Scene;
        }
        void AddSystem(SystemI* system)
        {
            m_Systems.push_back(system);
        }
        ~Hierarchy()
        {
            for (auto system : m_Systems)
            {
                delete system;
            }
        }
    private:
        Scene m_Scene;
        Node* m_Root;
        std::vector<SystemI*> m_Systems;
    };
}