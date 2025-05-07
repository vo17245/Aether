#pragma once
#include <Window/Layer.h>
#include <Window/Window.h>
#include "ApplicationResource.h"
#include "Text/Raster/Raster.h"
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
        std::string s="Hello, World!";
        ApplicationResource::GetSingleton().font->prepareGlyphsForText(s);
        std::vector<uint32_t> unicodes;
        for(auto c:s)
        {
            unicodes.push_back(c);
        }
        Text::Raster::RenderPassParam param{
            .commandBuffer=commandBuffer,
            .renderPass=renderPass,
            .frameBuffer=framebuffer,
            .descriptorPool=ApplicationResource::GetSingleton().descriptorPool,
            .font=*ApplicationResource::GetSingleton().font,
            .unicodes=unicodes,
            .glyphPosition={},
            .scale=1.0,
            .channel=0
        };
        ApplicationResource::GetSingleton().textRaster->Render(param,
        *ApplicationResource::GetSingleton().rasterResource);
        commandBuffer.EndRenderPass();
    }
    virtual void OnAttach(Window* window)override
    {
        m_Window=window;
    }
private:
    Window* m_Window;
};