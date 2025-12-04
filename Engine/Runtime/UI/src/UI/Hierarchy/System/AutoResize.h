#pragma once
#include "System.h"
#include "../Component/Base.h"
#include "../Component/AutoResize.h"
#include "../Component/Text.h"
#include <type_traits>
#include "Debug/Log.h"
#include "Window/WindowEvent.h"
#include "../Node.h"
#include <queue>
namespace Aether::UI
{
class AutoResizeSystem : public SystemI
{
public:
    using View = decltype(std::declval<Scene>().Select<BaseComponent, AutoResizeComponent>());
    struct Dispatcher
    {
        Scene& scene;
        Node* root;
        template <typename T>
        void operator()(T& event)
        {
        }
        template <>
        void operator()(WindowResizeEvent& event)
        {
            assert(root && "root is nullptr");
            std::queue<Node*> q;
            if (scene.HasComponent<AutoResizeComponent>(root->id))
            {
                float width = event.GetWidth();
                float height = event.GetHeight();
                auto& base = scene.GetComponent<BaseComponent>(root->id);
                auto& autoResize = scene.GetComponent<AutoResizeComponent>(root->id);
                if (autoResize.ResizeWidth())
                {
                    base.size.x() = width * autoResize.width;
                }
                if (autoResize.ResizeHeight())
                {
                    base.size.y() = height * autoResize.height;
                }
            }
            for (auto* child : root->children)
            {
                q.push(child);
            }
            while (!q.empty())
            {
                Node* node = q.front();
                q.pop();
                if (scene.HasComponent<AutoResizeComponent>(node->id))
                {
                    Node* parent = node->parent;
                    auto& parentBase = scene.GetComponent<BaseComponent>(parent->id);

                    auto& base = scene.GetComponent<BaseComponent>(node->id);
                    auto& autoResize = scene.GetComponent<AutoResizeComponent>(node->id);
                    // base.size = Vec2f(parentBase.size.x() * autoResize.width, parentBase.size.y() * autoResize.height);
                    if (autoResize.ResizeWidth())
                    {
                        base.size.x() = parentBase.size.x() * autoResize.width;
                    }
                    if (autoResize.ResizeHeight())
                    {
                        base.size.y() = parentBase.size.y() * autoResize.height;
                    }
                }

                for (auto* child : node->children)
                {
                    q.push(child);
                }
            }
        }
    };

public:
    virtual void OnEvent(Event& event, Scene& scene)
    {
        Dispatcher dispatcher{scene, m_Root};
        std::visit(dispatcher, event);
    }
    void SetRoot(Node* root)
    {
        m_Root = root;
    }

private:
    Vec2f m_MousePosition;
    Node* m_Root = nullptr;
};
} // namespace Aether::UI