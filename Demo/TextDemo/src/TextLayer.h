#pragma once
#include <Window/Layer.h>
#include <Window/Window.h>
using namespace Aether;
class TextLayer : public Layer
{
public:
    virtual void OnEvent(Event& event) override
    {
    }
    virtual void OnRender(
        vk::RenderPass& renderPass,
        vk::FrameBuffer& framebuffer,
        vk::GraphicsCommandBuffer& commandBuffer) override
    {
        commandBuffer.BeginRenderPass(renderPass,framebuffer,Vec4f(0,0,0,1));
        commandBuffer.SetScissor(0, 0, m_Window->GetSize().x(), m_Window->GetSize().y());
        commandBuffer.SetViewport(0, 0, m_Window->GetSize().x(), m_Window->GetSize().y());
        commandBuffer.EndRenderPass();
    }
    virtual void OnAttach(Window* window)override
    {
        m_Window=window;
    }
private:
    Window* m_Window;
};