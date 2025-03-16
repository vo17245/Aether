#include <Window/Window.h>
#include <Window/WindowContext.h>
#include <Render/RenderApi.h>
#include "Application.h"
using namespace Aether;
namespace Aether
{
extern Application* CreateApplication();
}

int main()
{
    auto* app=CreateApplication();
    WindowContext::Init();
    {
        auto window = std::unique_ptr<Window>(Window::Create(800, 600, "Hello, Aether"));
        // 会在window中创建vulkan对象,在销毁vulkan context前必须调用window 的ReleaseVulkanObjects 销毁window中的vulkan对象
        vk::GRC::Init(window.get(), true);
        app->OnInit(*window);
        while (!window->ShouldClose())
        {
            WindowContext::PollEvents();
            window->OnRender();
        }
        vkDeviceWaitIdle(vk::GRC::GetDevice());
        app->OnShutdown();
        window->ReleaseVulkanObjects();
        vk::GRC::Cleanup();
    }

    WindowContext::Shutdown();
    delete app;
}