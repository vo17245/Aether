#include <UI/UI.h>
#include "../Component/Drag.h"
#include <type_traits>
using namespace Aether;
class DragSystem:public UI::SystemI
{
public:
    using View=decltype(std::declval<Scene>().Select<UI::BaseComponent,DragComponent>());
    virtual void OnEvent(Event& event, Scene& scene) override
    {
        View view=scene.Select<UI::BaseComponent,DragComponent>();
        std::visit(Dispatcher{view,m_MousePosition},event);
    }
private:
    struct Dispatcher 
    {
        View& view;
        Vec2f& mousePosition;
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
                    base.position.x() = mousePosition.x() - base.size.x() / 2;
                    base.position.y() = mousePosition.y() - base.size.y() / 2;
                }
            }
        }   
    };
private:
    Vec2f m_MousePosition{0,0};

};