#pragma once
#include <Scene/Scene.h>
#include <memory>
#include <vector>
#include "Component/Base.h"
#include "LayoutBuilder.h"
#include "Node.h"
#include "Component/Node.h"
#include "Render/Scene/Camera2D.h"
#include "System/System.h"
#include <vector>
#include <queue>
#include <Window/Event.h>
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
    template <typename T, typename... Args>
    T& AddComponent(Node* node, Args&&... args)
    {
        auto& component = m_Scene.AddComponent<T>(node->id, std::forward<Args>(args)...);
        return component;
    }
    template <typename T>
    T& GetComponent(Node* node)
    {
        return m_Scene.GetComponent<T>(node->id);
    }
    Node* GetRoot()
    {
        return m_Root;
    }
    Scene& GetScene()
    {
        return m_Scene;
    }
    /**
     * @note system will be destroyed, when hierarchy destroy
     */
    void AddSystem(SystemI* system)
    {
        m_Systems.push_back(system);
    }
    Hierarchy()
    {
        InitCamera(Vec2f(1920, 1080));
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

    void RebuildLayout(const Vec2f& screenSize, float far)
    {
        m_LayoutBuilder.Begin(screenSize);
        struct NodeInfo
        {
            Node* node;
            size_t containerId;
            float containerZ;
        };
        std::queue<NodeInfo> nodes;
        nodes.push({m_Root, 0, far});
        size_t boxId = 0;
        while (!nodes.empty())
        {
            auto nodeInfo = nodes.front();
            nodes.pop();
            auto* node = nodeInfo.node;
            auto containerId = nodeInfo.containerId;
            auto& base = m_Scene.GetComponent<BaseComponent>(node->id);
            if (base.layoutEnabled)
            {
                base.position = m_LayoutBuilder.PushBox(base.size, containerId);
                base.z = nodeInfo.containerZ - 1;
            }
            else
            {
                m_LayoutBuilder.PlaceBox(base.size, base.position);
            }
            
            boxId++;
            for (auto& child : node->children)
            {
                nodes.push({child, boxId, base.z});
            }
        }
        m_LayoutBuilder.End();
    }
    void OnEvent(Event& event)
    {
        for (auto* system : m_Systems)
        {
            system->OnEvent(event, m_Scene);
        }
    }
    void OnUpdate(float sec)
    {
        for (auto* system : m_Systems)
        {
            system->OnUpdate(sec, m_Scene);
        }
    }
    void OnRender(DeviceCommandBufferView commandBuffer,
                  DeviceRenderPassView renderPass,
                  DeviceFrameBufferView frameBuffer,
                  Vec2f screenSize)
    {
        m_Camera->screenSize = screenSize;
        m_Camera->target.x() = screenSize.x() / 2;
        m_Camera->target.y() = screenSize.y() / 2;
        m_Camera->CalculateMatrix();
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
    Camera2D& GetCamera()
    {
        return *m_Camera;
    }

private:
    void InitCamera(const Vec2f& screenSize)
    {
        m_Camera = std::make_unique<Camera2D>();
        m_Camera->screenSize = screenSize;
        m_Camera->target = Vec2f(screenSize.x() / 2, screenSize.y() / 2);
        m_Camera->offset = Vec2f(0, 0);
        m_Camera->near = 0.0f;
        m_Camera->far = 10000.0f;
        m_Camera->zoom = 1.0f;
        m_Camera->rotation = 0.0f;
        m_Camera->CalculateMatrix();
    }

private:
    Scene m_Scene;
    Node* m_Root = nullptr;
    std::vector<SystemI*> m_Systems;
    LayoutBuilder m_LayoutBuilder;
    std::unique_ptr<Camera2D> m_Camera;
};
} // namespace Aether::UI