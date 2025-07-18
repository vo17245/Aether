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
#include "BuildLayout.h"
#include <Core/Borrow.h>
namespace Aether::UI
{
class Hierarchy
{
public:
    Node* CreateNode()
    {
        Node* node = new Node(this);
        node->id = m_Scene->CreateEntity();
        m_Scene->AddComponent<NodeComponent>(node->id, node);
        m_Scene->AddComponent<BaseComponent>(node->id, BaseComponent{Vec2f(0, 0),
                                                                     Vec2f(0, 0)});
        return node;
    }
    void DestroyNode(Node* node)
    {
        m_Scene->DestroyEntity(node->id);
        for (auto child : node->children)
        {
            DestroyNode(child);
        }
        delete node;
    }
    template <typename T, typename... Args>
    T& AddComponent(Node* node, Args&&... args)
    {
        auto& component = m_Scene->AddComponent<T>(node->id, std::forward<Args>(args)...);
        return component;
    }
    template <typename T>
    T& GetComponent(Node* node)
    {
        return m_Scene->GetComponent<T>(node->id);
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
    Hierarchy(Borrow<Scene> scene) :
        m_Scene(scene)
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

    void RebuildLayout()
    {
        BuildLayout(m_Root, m_Scene, m_Camera->screenSize, m_Camera->far, m_LayoutBuilder);
    }
    void OnEvent(Event& event)
    {
        for (auto* system : m_Systems)
        {
            system->OnEvent(event, m_Scene);
        }
        if (std::holds_alternative<WindowResizeEvent>(event))
        {
            auto& resizeEvent = std::get<WindowResizeEvent>(event);
            if (resizeEvent.GetHeight() == 0 || resizeEvent.GetWidth() == 0)
            {
            }
            else
            {
                m_Camera->screenSize.x() = resizeEvent.GetWidth();
                m_Camera->screenSize.y() = resizeEvent.GetHeight();
                RebuildLayout();
            }
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
                  DeviceFrameBufferView frameBuffer,
                  Vec2f screenSize)
    {
        m_Camera->screenSize = screenSize;
        m_Camera->target.x() = screenSize.x() / 2;
        m_Camera->target.y() = screenSize.y() / 2;
        m_Camera->CalculateMatrix();
        for (auto* system : m_Systems)
        {
            system->OnRender(commandBuffer, frameBuffer, screenSize, m_Scene);
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
    void OnFrameBegin()
    {
        if (m_NeedRebuildLayout)
        {
            RebuildLayout();
            m_NeedRebuildLayout = false;
        }
    }
    void RequireRebuildLayout()
    {
        m_NeedRebuildLayout = true;
    }
    std::unordered_map<std::string, Node*>& GetNodeIdMap()
    {
        return m_NodeId;
    }
    const std::unordered_map<std::string, Node*>& GetNodeIdMap() const
    {
        return m_NodeId;
    }
    Node* GetNodeById(const std::string& id) const
    {
        auto iter = m_NodeId.find(id);
        if (iter != m_NodeId.end())
        {
            return iter->second;
        }
        return nullptr;
    }
    void SetScene(Borrow<Scene> scene)
    {
        m_Scene = scene;
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
    Borrow<Scene> m_Scene;
    Node* m_Root = nullptr;
    std::vector<SystemI*> m_Systems;
    LayoutBuilder m_LayoutBuilder;
    std::unique_ptr<Camera2D> m_Camera;
    bool m_NeedRebuildLayout = false; // 是否需要重新布局
    std::unordered_map<std::string, Node*> m_NodeId;
};
template <typename T>
inline T& Node::GetComponent()
{
    return hierarchy->GetComponent<T>(this);
}
template <typename T>
inline bool Node::HasComponent()
{
    return hierarchy->GetScene().HasComponent<T>(id);
}
template <typename T, typename... Args>
inline T& Node::AddComponent(Args&&... args)
{
    return hierarchy->AddComponent<T>(this, std::forward<Args>(args)...);
}
} // namespace Aether::UI