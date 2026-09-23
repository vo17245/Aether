#include <Window/Window.h>
#include <Window/WindowContext.h>
#include "Application.h"
#include <Audio/Audio.h>
#include <ImGui/Compat/ImGuiApi.h>
#include <Debug/Log.h>
#include <Async/GlobalThreadPool.h>
#include <Render/Threads/RenderThread.h>
#include <MainLoop/MainLoop.h>
#include <stdexcept>
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
        if (!task)
        {
            LogE("GlobalThreadPool returned an empty completion");
            continue;
        }
        task->OnComplete();
    }
}
} // namespace
int main()
{
    auto app = std::unique_ptr<Application>(CreateApplication());
    if (!app) return -1;
    const auto initParams = app->GetInitParams();
    std::unique_ptr<Window> window;
    bool threadPoolInitialized = false;
    bool windowContextInitialized = false;
    bool audioInitialized = false;
    bool renderContextInitialized = false;
    bool renderObjectsCreated = false;
    bool renderThreadInitialized = false;
    bool imGuiInitialized = false;
    bool applicationInitialized = false;
    bool shutdownStarted = false;
    std::unique_ptr<Render::RenderThread> renderThread;

    auto cleanupStep = [](const char* name, auto&& callback) {
        try
        {
            callback();
        }
        catch (const std::exception& exception)
        {
            LogE("Cleanup step '{}' failed: {}", name, exception.what());
        }
        catch (...)
        {
            LogE("Cleanup step '{}' failed with an unknown exception", name);
        }
    };

    auto shutdown = [&] {
        if (shutdownStarted) return;
        shutdownStarted = true;
        if (threadPoolInitialized)
        {
            GlobalThreadPool::StopAccepting();
            GlobalThreadPool::JoinWorkers();
            cleanupStep("background completions", HandleGlobalThreadPoolCompleteTasks);
        }
        if (applicationInitialized)
            cleanupStep("application shutdown", [&] { app->OnShutdown(); });
        applicationInitialized = false;
        if (window && window->HasLayers())
            cleanupStep("layer detach", [&] { window->DetachAllLayers(); });
        if (imGuiInitialized)
            cleanupStep("ImGui renderer frontend release", ImGuiApi::PrepareRendererShutdown);
        if (window)
        {
            if (renderObjectsCreated)
                cleanupStep("window rendering shutdown", [&] { window->ShutdownRendering(); });
        }
        renderObjectsCreated = false;
        if (renderThreadInitialized && renderThread)
        {
            renderThread->RequestStop();
            renderThread->Join();
        }
        renderThreadInitialized = false;
        renderThread.reset();
        if (imGuiInitialized)
            cleanupStep("ImGui frontend shutdown", ImGuiApi::ShutdownFrontend);
        imGuiInitialized = false;
        if (threadPoolInitialized) GlobalThreadPool::Destory();
        threadPoolInitialized = false;
        app.reset();
        if (window && renderContextInitialized)
            cleanupStep("window surface release", [&] { window->CleanupSurface(); });
        window.reset();
        if (renderContextInitialized) vk::GRC::Cleanup();
        renderContextInitialized = false;
        if (audioInitialized) Audio::Destory();
        audioInitialized = false;
        if (windowContextInitialized) WindowContext::Shutdown();
        windowContextInitialized = false;
    };

    try
    {
        if (initParams.enableGlobalThreadPool)
        {
            GlobalThreadPool::Init(initParams.globalThreadPoolThreadCount);
            threadPoolInitialized = true;
        }
        if (!WindowContext::Init())
            throw std::runtime_error("failed to initialize SDL3 window context");
        windowContextInitialized = true;
        if (Audio::Init() != 0)
            throw std::runtime_error("failed to initialize audio");
        audioInitialized = true;
        window.reset(Window::Create(app->MainWindowCreateParam()));
        if (!window)
            throw std::runtime_error("failed to create the main window");
        // Window/surface must be released before the Vulkan context.
        vk::RenderContext::Config config;
        config.enableValidationLayers = true;
        config.enableDynamicRendering = true;
        vk::InitResource initResource;
        initResource.instanceExtensions = WindowContext::RequiredVulkanInstanceExtensions();
        initResource.createSurface = [windowPtr = window.get()](VkInstance instance) {
            if (windowPtr->CreateSurface(instance) != VK_SUCCESS)
                return VkSurfaceKHR(VK_NULL_HANDLE);
            return windowPtr->GetSurface();
        };
        // GRC exposes partial handles for reverse-order cleanup if Init throws.
        renderContextInitialized = true;
        vk::GRC::Init(initResource, config);
        ImGuiApi::InitFrontend(*window);
        imGuiInitialized = true;
        renderThread = std::make_unique<Render::RenderThread>(
            Render::RenderThreadLimits{}, [] { vk::GRC::BindRuntimeRenderThread(); },
            [windowPtr = window.get()] {
                try
                {
                    windowPtr->CleanupRenderingOnRenderThread();
                    vk::GRC::CleanupCurrentThreadResources();
                }
                catch (...)
                {
                    vk::GRC::CleanupCurrentThreadResources();
                    vk::GRC::UnbindRuntimeRenderThread();
                    throw;
                }
                vk::GRC::UnbindRuntimeRenderThread();
            });
        if (!renderThread->Start())
            throw std::runtime_error("failed to start RenderThread");
        renderThreadInitialized = true;
        if (renderThread->ReadyTicket().Wait() != Render::TicketWaitStatus::Completed)
            throw std::runtime_error("RenderThread startup did not complete");
        const auto ready = renderThread->ReadyTicket().TryGetResult();
        if (!ready || ready->status != Render::CommandCompletionStatus::Succeeded)
            throw std::runtime_error(ready ? ready->message : "RenderThread startup result is unavailable");
        window->InitializeRendering(*renderThread);
        renderObjectsCreated = true;
        applicationInitialized = true;
        app->OnInit(*window);
        MainLoop::OnFrameBegin += [&]() {
            if (window->ShouldClose() || !app->Running())
            {
                MainLoop::Quit();
                return;
            }
            app->OnFrameBegin();
            if (initParams.enableGlobalThreadPool) HandleGlobalThreadPoolCompleteTasks();
        };
        MainLoop::OnEvent += [&]() {
            WindowContext::PollEvents();
            window->DispatchEvent();
            if (window->ShouldClose() || !app->Running()) MainLoop::Quit();
        };
        MainLoop::OnUpdate += [&](float deltaSec) { window->OnUpdate(deltaSec); };
        MainLoop::OnRender += [&]() { window->OnRender(); };
        MainLoop::OnCleanup += shutdown;
        MainLoop::Run();
        return 0;
    }
    catch (const std::exception& exception)
    {
        LogE("Fatal initialization/runtime error: {}", exception.what());
        shutdown();
        return -1;
    }
    catch (...)
    {
        LogE("Fatal initialization/runtime error: unknown exception");
        shutdown();
        return -1;
    }
}
