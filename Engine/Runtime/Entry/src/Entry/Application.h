#pragma once
#include <Window/Window.h>
#include "InitParams.h"
#include <span>
#include <string_view>
namespace Aether
{
    class Application
    {
    public:
        virtual ~Application() = default;
        // Called before engine initialization. Arguments exclude the executable name
        // and remain valid until shutdown. Return false to exit with status 1.
        virtual bool OnCommandLineArguments(std::span<const std::string_view> arguments)
        {
            return true;
        }
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
        virtual InitParams GetInitParams()const;
    private:
        bool m_Running=true;
    };
}

#define DEFINE_APPLICATION(cls) namespace Aether{Application* CreateApplication(){return new cls();}}