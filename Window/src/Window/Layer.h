#pragma once
#include "Render/Vulkan/GraphicsCommandBuffer.h"
#include "Window/Event.h"
#include "Render/Vulkan/RenderPass.h"
#include "Render/Vulkan/FrameBuffer.h"
#include "Render/Vulkan/Semaphore.h"
#include "Render/Vulkan/Fence.h"
namespace Aether {
class Window;
class Layer
{
public:
    virtual ~Layer() = default;
    Layer() = default;
    // 挂载的window负责调用OnAttach和OnDetach，并把自己传给Layer
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
} // namespace Aether