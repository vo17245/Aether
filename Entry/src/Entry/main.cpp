#include <Window/Window.h>
#include <Window/WindowContext.h>
#include <Render/RenderApi.h>
#include "Application.h"
#include <chrono>
#include <Audio/Audio.h>
using namespace Aether;
namespace Aether
{
extern Application* CreateApplication();
}

int main()
{
    auto* app=CreateApplication();
    WindowContext::Init();
    if(Audio::Init()!=0)
    {
        assert(false&&&"Audio Init Failed");
        return -1;
    }
    {
        auto window = std::unique_ptr<Window>(Window::Create(800, 600, app->GetName()));
        // 会在window中创建vulkan对象,在销毁vulkan context前必须调用window 的ReleaseVulkanObjects 销毁window中的vulkan对象
        vk::GRC::Init(window.get(), true);
        WindowContext::Register(window->GetHandle(), window.get());
        app->OnInit(*window);
        std::chrono::high_resolution_clock::time_point lastTime = std::chrono::high_resolution_clock::now();
        while (!window->ShouldClose())
        {
            //window->WaitLastFrameComplete();
            app->OnFrameBegin();

            WindowContext::PollEvents();
            window->DispatchEvent();

            window->OnRender();
            std::chrono::high_resolution_clock::time_point currentTime = std::chrono::high_resolution_clock::now();
            float sec=
                std::chrono::duration<float>(currentTime - lastTime).count();
            lastTime = currentTime;
            window->OnUpdate(sec);
        }
        vkDeviceWaitIdle(vk::GRC::GetDevice());
        app->OnShutdown();
        window->ReleaseVulkanObjects();
        vk::GRC::Cleanup();
    }
    Audio::Destory();
    WindowContext::Shutdown();
    delete app;
}