#include "TestLayer.h"
#include <iostream>
int main()
{
    WindowContext::Init();
    {
        auto window = std::unique_ptr<Window>(Window::Create(800, 600, "Hello, Aether"));
        // 会在window中创建vulkan对象,在销毁vulkan context前必须调用window 的ReleaseVulkanObjects 销毁window中的vulkan对象
        vk::GRC::Init(window.get(), true);
        auto* testLayer = new TestLayer();
        window->PushLayer(testLayer);
        while (!window->ShouldClose())
        {
            WindowContext::PollEvents();
            window->OnRender();
        }
        vkDeviceWaitIdle(vk::GRC::GetDevice());
        delete testLayer;
        window->ReleaseVulkanObjects();
        vk::GRC::Cleanup();
    }

    WindowContext::Shutdown();
}