#pragma once
#include <Window/Window.h>
#include <Window/Event.h>
using namespace Aether;
class Test
{
public:
    virtual ~Test() = default;
    Test() = default;
    virtual void OnAttach(Window* window)
    {
    }
    virtual void OnDetach()
    {
    }
    virtual void OnUpdate(float sec)
    {
    }
    virtual void OnEvent(Event& e)
    {
    }

    virtual void OnRender(
        vk::RenderPass& renderPass,
        vk::FrameBuffer& framebuffer,
        vk::GraphicsCommandBuffer& commandBuffer
    )
    {
    }
    virtual void OnFrameBegin()
    {
    }
   
};