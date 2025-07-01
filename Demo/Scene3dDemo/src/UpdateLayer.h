#pragma once
#include "Core/Math/Def.h"

#include "Window/Layer.h"
#include "Window/Window.h"
#include "Window/WindowContext.h"


#include "Resource/Finder.h"
#include "Window/WindowEvent.h"
#include "ApplicationResource.h"

using namespace Aether;
class UpdateLayer : public Layer
{
public:
    
    virtual void OnAttach(Window* window)override
    {
        m_Window = window;
    }
    virtual void OnEvent(Event& event) override
    {
        if(std::holds_alternative<WindowResizeEvent>(event))
        {
            auto& resizeEvent = std::get<WindowResizeEvent>(event);
            ApplicationResource::s_Instance->ResizeHierarchyFrameBuffer(m_Window->GetSize(),m_Window->GetFinalTexture());
        }
    }
private:
    Window* m_Window = nullptr;
};
