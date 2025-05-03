#pragma once
#include <Scene/Scene.h>
#include <vector>
#include "Component/Base.h"
#include "LayoutBuilder.h"
#include "Node.h"
#include "Component/Node.h"
#include "System/System.h"
#include <vector>
#include <queue>
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
        m_Scene.AddComponent<BaseComponent>(node->id, BaseComponent{Vec2f(0, 0),
                                                                    Vec2f(0, 0)});
        return node;
    }
    void DestroyNode(Node* node)
    {
        m_Scene.DestroyEntity(node->id);
        for (auto child : node->children)
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
        if (m_Root)
        {
            DestroyNode(m_Root);
        }
        for (auto system : m_Systems)
        {
            delete system;
        }
    }

    void RebuildLayout(const Vec2f& screenSize,float far)
    {
        m_LayoutBuilder.Begin(screenSize);
        struct NodeInfo
        {
            Node* node;
            size_t containerId;
            float containerZ;
        };
        std::queue<NodeInfo> nodes;
        nodes.push({m_Root, 0,far});
        size_t boxId = 0;
        while (!nodes.empty())
        {
            auto nodeInfo = nodes.front();
            nodes.pop();
            auto* node = nodeInfo.node;
            auto containerId = nodeInfo.containerId;
            auto& base = m_Scene.GetComponent<BaseComponent>(node->id);

            base.position = m_LayoutBuilder.PushBox(base.size, containerId);
            base.z = nodeInfo.containerZ-1;
            boxId++;
            for (auto& child : node->children)
            {
                nodes.push({child, boxId, base.z});
            }
        }
        m_LayoutBuilder.End();
    }
    void OnUpdate(float sec, Scene& scene)
    {
        for (auto* system : m_Systems)
        {
            system->OnUpdate(sec, scene);
        }
    }
    void OnRender(DeviceCommandBufferView commandBuffer,
                  DeviceRenderPassView renderPass,
                  DeviceFrameBufferView frameBuffer,
                  Vec2f screenSize)
    {
        for (auto* system : m_Systems)
        {
            system->OnRender(commandBuffer, renderPass, frameBuffer, screenSize, m_Scene);
        }
    }
    /**
     * @note Hierarchy will destroy the root node when destroyed
     */
    void SetRoot(Node* root)
    {
        m_Root = root;
    }

private:
    Scene m_Scene;
    Node* m_Root = nullptr;
    std::vector<SystemI*> m_Systems;
    LayoutBuilder m_LayoutBuilder;
};
} // namespace Aether::UI