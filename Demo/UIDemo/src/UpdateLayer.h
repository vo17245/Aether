#pragma once
#include <Window/Layer.h>
#include <Window/Window.h>
using namespace Aether;
#include "ApplicationResource.h"
class UpdateLayer:public Layer
{
public:
    virtual void OnEvent(Event& event) override
    {
        if (std::holds_alternative<WindowResizeEvent>(event))
        {
            // update camera
            auto& camera=ApplicationResource::s_Instance->camera;
            auto& e = std::get<WindowResizeEvent>(event);
            auto& renderer= *ApplicationResource::s_Instance->renderer;
            camera.screenSize.x() = e.GetWidth();
            camera.screenSize.y() = e.GetHeight();
            // update hierarchy framebuffer
            ApplicationResource::s_Instance->ResizeHierarchyFrameBuffer(Vec2i(e.GetWidth(),e.GetHeight()),
             m_Window->GetFinalTexture());
        }
    }
    virtual void OnAttach(Window* window)override
    {
        m_Window=window;
    }
private:
    Window* m_Window=nullptr;
};