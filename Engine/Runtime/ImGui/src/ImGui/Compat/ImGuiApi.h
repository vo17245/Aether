#pragma once
namespace Aether
{
    class Window;
}
namespace Aether::ImGuiApi
{
    void NewFrame();
    void Shutdown();
    void Init(Window& window);
    void InitFrontend(Window& window);
    void ShutdownFrontend();
    void PrepareRendererShutdown();
    void EnableDocking();
    void SwitchToMainContext();
}
