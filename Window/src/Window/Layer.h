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
        vk::Semaphore& wait,   // gpu上等待framebuffer可用
        vk::Semaphore& signal, // gpu上通知framebuffer已经渲染完毕
        vk::Fence& fence,      // cpu上通知framebuffer已经渲染完毕
        vk::Fence& lastFence,//layer渲染时需要等待上一帧结束
        uint32_t currentFrame // fence是调用者的第几个fence
    )
    {
    }
    virtual void OnLoopEnd()
    {
    }
    virtual void OnLoopBegin()
    {
    }
};
} // namespace Aether