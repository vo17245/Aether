#pragma once
#include <Window/Layer.h>
using namespace Aether;
#include "ApplicationResource.h"
class UpdateLayer:public Layer
{
public:
    virtual void OnEvent(Event& event) override
    {
        if (std::holds_alternative<WindowResizeEvent>(event))
        {
            auto& e = std::get<WindowResizeEvent>(event);
            auto& renderer= *ApplicationResource::s_Instance->renderer;
            renderer.GetCamera().screenSize.x() = e.GetWidth();
            renderer.GetCamera().screenSize.y() = e.GetHeight();
        }
    }
};