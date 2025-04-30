#pragma once
#include <Window/Window.h>
namespace Aether
{
    class Application
    {
    public:
        virtual void OnInit(Window& window)
        {
            // e.q. push layer
        }
        virtual void OnShutdown()
        {
            //e.q. pop layer and destory layer 
        }
        virtual void OnFrameBegin()
        {
            // e.q. clear last frame temporary resource
        }
        virtual const char* GetName()const
        {
            return "Aether Application";
        }
    };
}

#define DEFINE_APPLICATION(cls) namespace Aether{Application* CreateApplication(){return new cls();}}