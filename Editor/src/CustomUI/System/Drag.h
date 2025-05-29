#include <UI/UI.h>
#include "../Component/Drag.h"
#include "Render/Scene/Camera2D.h"
#include "UI/Hierarchy/Hierarchy.h"
using namespace Aether;
class DragSystem:public UI::SystemI
{
public:
    using View=decltype(std::declval<Scene>().Select<UI::BaseComponent,DragComponent>());
    virtual void OnEvent(Event& event, Scene& scene) override
    {
        View view=scene.Select<UI::BaseComponent,DragComponent>();
        std::visit(Dispatcher{view,m_MousePosition,*m_Hierarchy},event);
    }
    void SetHierarchy(UI::Hierarchy* hierarchy)
    {
        m_Hierarchy = hierarchy;
    }
private:
    struct Dispatcher 
    {
        View& view;
        Vec2f& mousePosition;
        UI::Hierarchy& hierarchy;
        template<typename T>
        void operator()(T& event)
        {

        }
        template<>
        void operator()(MouseButtonPressedEvent& event)
        {
            for(const auto& [entity,base,drag]:view.each())
            {
                if (mousePosition.x() >= base.position.x() 
                && mousePosition.x() <= base.position.x() + base.size.x() 
                && mousePosition.y() >= base.position.y() 
                && mousePosition.y() <= base.position.y() + base.size.y())
                {
                    drag.isDragging = true;
                    drag.offset=mousePosition-base.position;
                }
            }
        }
        template<>
        void operator()(MouseButtonReleasedEvent& event)
        {
            for(const auto& [entity,base,drag]:view.each())
            {
                if (drag.isDragging)
                {
                    drag.isDragging = false;
                }
            }
        }
        template<>
        void operator()(MousePositionEvent& event)
        {
            mousePosition=event.GetPosition();
            for(const auto& [entity,base,drag]:view.each())
            {
                if (drag.isDragging)
                {
                    base.position=mousePosition-drag.offset;
                    hierarchy.RequireRebuildLayout();
                }
            }
        }   
    };
    

private:
    Vec2f m_MousePosition{0,0};
    UI::Hierarchy* m_Hierarchy;
};