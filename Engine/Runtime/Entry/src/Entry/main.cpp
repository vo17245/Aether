#include <Window/Window.h>
#include <Window/WindowContext.h>
#include <Render/RenderApi.h>
#include "Application.h"
#include <chrono>
#include <Audio/Audio.h>
#include <ImGui/Compat/ImGuiApi.h>
#include <Debug/Log.h>
#include <Async/GlobalThreadPool.h>
#include <Render/NamedThread/SubmitThread.h>
#include <MainLoop/MainLoop.h>
using namespace Aether;
namespace Aether
{
extern Application* CreateApplication();
}
namespace
{
void HandleGlobalThreadPoolCompleteTasks()
{
    auto& queue = GlobalThreadPool::GetCompleteQueue();
    Scope<TaskBase> task;
    while (queue.try_dequeue(task))
    {
        assert(true == static_cast<bool>(task));
        task->OnComplete();
    }
}
} // namespace
int main()
{
    auto* app = CreateApplication();
    auto initParams = app->GetInitParams();
    if (initParams.enableGlobalThreadPool)
    {
        GlobalThreadPool::Init(initParams.globalThreadPoolThreadCount);
    }
    WindowContext::Init();
    if (Audio::Init() != 0)
    {
        assert(false && &"Audio Init Failed");
        return -1;
    }
    auto window = std::unique_ptr<Window>(Window::Create(app->MainWindowCreateParam()));
    // 会在window中创建vulkan对象,在销毁vulkan context前必须调用window 的ReleaseVulkanObjects
    // 销毁window中的vulkan对象
    vk::RenderContext::Config config;
    config.enableValidationLayers = true;
    config.enableDynamicRendering = false;
    vk::GRC::Init(window.get(), config);
    Render::SubmitThread::Init();
    WindowContext::Register(window->GetHandle(), window.get());
    window->ImGuiWindowContextInit();
    ImGuiApi::Init(*window);
    app->OnInit(*window);
    std::chrono::high_resolution_clock::time_point lastTime = std::chrono::high_resolution_clock::now();
    MainLoop::OnFrameBegin += [&]() {
        if (!window->ShouldClose() && app->Running())
        {
            MainLoop::Quit();
        }
        app->OnFrameBegin();
        if (initParams.enableGlobalThreadPool)
        {
            HandleGlobalThreadPoolCompleteTasks();
        }
    };
    MainLoop::OnEvent += [&]() {
        WindowContext::PollEvents();
        window->DispatchEvent();
    };
    MainLoop::OnUpdate += [&](float deltaSec) { window->OnUpdate(deltaSec); };
    MainLoop::OnUpload += [&]() { window->OnUpload(); };
    MainLoop::OnRender += [&]() { window->OnRender(); };
    MainLoop::OnCleanup += [&]() {
        Render::SubmitThread::Shutdown();
        vkDeviceWaitIdle(vk::GRC::GetDevice());
        app->OnShutdown();
        window->ReleaseVulkanObjects();
        window->ImGuiWindowContextDestroy();
        delete app;
        window.reset();
        ImGuiApi::Shutdown();
        vk::GRC::Cleanup();
        window.reset();
        Audio::Destory();
        WindowContext::Shutdown();
    };
    MainLoop::Run();
    return 0;
}