#pragma once
#include "../Component/Mouse.h"
#include "../Component/Base.h"
#include "Window/Event.h"
#include "System.h"
#include "UI/Hierarchy/Component/Node.h"
namespace Aether::UI
{
class MouseSystem:public SystemI
{
public:
    void OnUpdate(float sec, Scene& scene)
    {
        auto view = scene.Select<NodeComponent, BaseComponent, MouseComponent>();
        for (const auto& [entity, node, base, mouse] : view.each())
        {
        }
    }
    void OnEvent(Event& event, Scene& scene)
    {
        auto view = scene.Select<NodeComponent, BaseComponent, MouseComponent>();

        if (std::holds_alternative<MousePositionEvent>(event))
        {
            auto& mousePosEvent = std::get<MousePositionEvent>(event);
            auto& pos = mousePosEvent.GetPosition();

            for (const auto& [entity, node, base, mouse] : view.each())
            {
                if (pos.x() >= base.position.x() && pos.x() <= base.position.x() + base.size.x() && pos.y() >= base.position.y() && pos.y() <= base.position.y() + base.size.y())
                {
                    mouse.isHover = true;
                    if (mouse.onHover)
                        mouse.onHover();
                }
                else
                {
                    mouse.isHover = false;
                }
            }
        }
    }
};
} // namespace Aether::UI