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

    void MainLoop::Run()
    {
        isRunning = true;
        OnStart.Broadcast();
        auto lastTime = std::chrono::steady_clock::now();

        while (isRunning)
        {
            OnFrameBegin.Broadcast();
            if (!isRunning) break;
            OnEvent.Broadcast();
            if (!isRunning) break;
            const auto currentTime = std::chrono::steady_clock::now();
            float deltaSec = std::chrono::duration<float>(currentTime - lastTime).count();
            lastTime = currentTime;
            OnUpdate.Broadcast(deltaSec);
            if (!isRunning) break;
            OnUpload.Broadcast();
            if (!isRunning) break;
            OnRender.Broadcast();
        }
        OnCleanup.Broadcast();
        OnQuit.Broadcast();
    }

    void MainLoop::Quit()
    {
        isRunning = false;
    }

    bool MainLoop::IsRunning()
    {
        return isRunning;
    }
}
