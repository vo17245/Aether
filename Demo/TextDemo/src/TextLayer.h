#pragma once
#include <Window/Layer.h>
#include <Window/Window.h>
#include "ApplicationResource.h"
#include "Text/Raster/Raster.h"
#include <Core/String.h>
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
        float framePerSec = m_Frame/m_Sec;
        std::string s=std::to_string(framePerSec)+"   1234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
        ApplicationResource::GetSingleton().font->prepareGlyphsForText(s);
        U32String u32s(s);
        std::vector<uint32_t> unicodes;
        for(size_t i=0;i<u32s.Size();i++)
        {
            unicodes.push_back(u32s[i]);
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
    virtual void OnUpdate(float sec)override
    {
        m_Sec+=sec;
        m_Frame++;
    }
private:
    Window* m_Window;
    float m_Sec=0;
    uint32_t m_Frame=0;
};