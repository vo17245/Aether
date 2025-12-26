#include "MainLoop.h"
#include <chrono>

namespace Aether
{
    Delegate<void()> MainLoop::OnStart;
    Delegate<void(float)> MainLoop::OnUpdate;
    Delegate<void()> MainLoop::OnQuit;
    Delegate<void()> MainLoop::OnRender;
    Delegate<void()> MainLoop::OnFrameBegin;
    Delegate<void()> MainLoop::OnUpload;
    Delegate<void()> MainLoop::OnEvent;
    Delegate<void()> MainLoop::OnCleanup;
    static bool isRunning = false;
    static std::chrono::high_resolution_clock::time_point lastTime;

    void MainLoop::Run()
    {
        isRunning = true;
        OnStart.Broadcast();

        while (isRunning)
        {
            OnEvent.Broadcast();
            auto currentTime = std::chrono::high_resolution_clock::now();
            float deltaSec = std::chrono::duration<float>(currentTime - lastTime).count();
            lastTime = currentTime;
            OnUpdate.Broadcast(deltaSec);
            OnUpload.Broadcast();
            OnRender.Broadcast();
        }
        OnQuit.Broadcast();
    }

    void MainLoop::Quit()
    {
        isRunning = false;
    }
}