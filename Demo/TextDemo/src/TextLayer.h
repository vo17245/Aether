#pragma once
#include <Window/Layer.h>
#include <Window/Window.h>
#include "ApplicationResource.h"
#include "Text/Raster/Raster.h"
#include "Window/WindowEvent.h"
#include <Core/String.h>
#include <variant>
using namespace Aether;
class TextLayer : public Layer
{
public:
    virtual void OnEvent(Event& event) override
    {
        if(std::holds_alternative<WindowResizeEvent>(event))
        {
            auto& resizeEvent=std::get<WindowResizeEvent>(event);

            auto& camera=ApplicationResource::GetSingleton().camera;
            camera.screenSize.x()=resizeEvent.GetWidth();
            camera.screenSize.y()=resizeEvent.GetHeight();
            camera.target.x()=resizeEvent.GetWidth()/2.0;
            camera.target.y()=resizeEvent.GetHeight()/2.0;
        }
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
        std::string s=std::string("")+"fps: "+std::to_string(framePerSec)+"\n"
        +"我能吞下玻璃而不伤身体\n"
        +"0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ\n"
        
        ;
        // get glyph curve
        ApplicationResource::GetSingleton().font->prepareGlyphsForText(s);
        
        U32String u32s(s);
        std::vector<uint32_t> unicodes;
        for(size_t i=0;i<u32s.Size();i++)
        {
            unicodes.push_back(u32s[i]);
        }
        // calculate glyph layout
        std::vector<Vec2f> glyphPos;
        glyphPos.reserve(unicodes.size());
        float worldSize=32.0f;
        {
            float scale=worldSize/ApplicationResource::GetSingleton().font->emSize;
            float x=0;
            float y=0;
            float width=700;
            float emSize=ApplicationResource::GetSingleton().font->emSize;
            for(auto& unicode:unicodes)
            {
                auto& glyph=ApplicationResource::GetSingleton().font->glyphs[unicode];
                if(unicode=='\n')
                {
                    x=0;
                    y+=worldSize;
                    glyphPos.push_back(Vec2f(0,0));
                    continue;
                }
                glyphPos.push_back(Vec2f(x,y+(emSize-glyph.bearingY)*scale));
                x+=(glyph.advance+glyph.kerningX)*scale;
                if(x>width)
                {
                    x=0;
                    y+=worldSize;
                }
            }
        }
        auto& camera=ApplicationResource::GetSingleton().camera;
        camera.CalculateMatrix();
        Text::Raster::RenderPassParam param{
            .commandBuffer=commandBuffer,
            .renderPass=renderPass,
            .frameBuffer=framebuffer,
            .descriptorPool=ApplicationResource::GetSingleton().descriptorPool,
            .font=*ApplicationResource::GetSingleton().font,
            .unicodes=unicodes,
            .glyphPosition=glyphPos,
            .worldSize=worldSize,
            .camera=camera,
            .z=0.0f
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