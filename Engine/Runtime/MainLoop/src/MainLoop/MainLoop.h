#pragma once
#include <Core/Delegate.h>
namespace Aether
{
    class MainLoop
    {
    public:
        static void Run();
        static void Quit();
        static Delegate<void()> OnStart;
        static Delegate<void(float/*deltaSec*/)> OnUpdate;
        static Delegate<void()> OnQuit;
        static Delegate<void()> OnRender;
        static Delegate<void()> OnEvent;
        static Delegate<void()> OnFrameBegin;
        static Delegate<void()> OnUpload;
        static Delegate<void()> OnCleanup;

    };
}