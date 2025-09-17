#pragma once
#include <Window/Window.h>
namespace Aether
{
    class Application
    {
    public:
        virtual ~Application() = default;
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
        }
        virtual const char* GetName()const
        {
            return "Aether Application";
        }
        bool Running() const
        {
            return m_Running;
        }
        void Quit()
        {
            m_Running = false;
        }
        virtual WindowCreateParam MainWindowCreateParam()
        {
            auto param=WindowCreateParam{};
            return param;
        }
    private:
        bool m_Running=true;
    };
}

#define DEFINE_APPLICATION(cls) namespace Aether{Application* CreateApplication(){return new cls();}}