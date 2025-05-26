#pragma once
#include "../Component/Mouse.h"
#include "../Component/Base.h"
#include "Window/Event.h"
#include "System.h"
#include "UI/Hierarchy/Component/Node.h"
#include "Window/MouseEvent.h"
#include <variant>
namespace Aether::UI
{
class MouseSystem : public SystemI
{
public:
    virtual void OnEvent(Event& event, Scene& scene)override
    {
        auto view = scene.Select<BaseComponent, MouseComponent>();
        // hover
        if (std::holds_alternative<MousePositionEvent>(event))
        {
            auto& mousePosEvent = std::get<MousePositionEvent>(event);
            auto& pos = mousePosEvent.GetPosition();
            m_MousePosition = pos;
            for (const auto& [entity, base, mouse] : view.each())
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
        else if(std::holds_alternative<MouseButtonPressedEvent>(event))
        {
            auto& mouseButtonEvent = std::get<MouseButtonPressedEvent>(event);
            auto buttonCode = mouseButtonEvent.GetKeyCode();
            auto& pos= m_MousePosition;
            for (const auto& [entity, base, mouse] : view.each())
            {
                if (pos.x() >= base.position.x() && pos.x() <= base.position.x() + base.size.x() && pos.y() >= base.position.y() && pos.y() <= base.position.y() + base.size.y())
                {
                    if(mouse.onPress)
                        mouse.onPress();
                    mouse.isPress = true;
                }
                else
                {
                    mouse.isPress = false;
                }
            }
        }
        else if(std::holds_alternative<MouseButtonReleasedEvent>(event))
        {
            auto& mouseButtonEvent = std::get<MouseButtonReleasedEvent>(event);
            auto buttonCode = mouseButtonEvent.GetKeyCode();
            auto& pos= m_MousePosition;
            for (const auto& [entity, base, mouse] : view.each())
            {
                if (pos.x() >= base.position.x() && pos.x() <= base.position.x() + base.size.x() && pos.y() >= base.position.y() && pos.y() <= base.position.y() + base.size.y())
                {
                    if(mouse.isPress)
                    {
                        if (mouse.onClick)
                            mouse.onClick();
                    }
                    if (mouse.onRelease)
                        mouse.onRelease();
                    mouse.isPress = false;
                    
                }
            }
        }
      

    }
private:
    Vec2f m_MousePosition = Vec2f(0, 0);

};
} // namespace Aether::UI