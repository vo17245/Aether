#pragma once
#include <Window/Layer.h>
#include <Window/Window.h>
#include "ApplicationResource.h"
#include "Text/Raster/Raster.h"
#include "Window/WindowEvent.h"
#include <Core/String.h>
#include <variant>
#include <Render/TaskGraph.h>
#include "DownsamplingRenderPass.h"
using namespace Aether;
class TextLayer : public Layer
{
public:
    virtual void OnEvent(Event& event) override
    {
        if (std::holds_alternative<WindowResizeEvent>(event))
        {
            auto& resizeEvent = std::get<WindowResizeEvent>(event);

            auto& camera = ApplicationResource::GetSingleton().camera;
            camera.screenSize.x() = resizeEvent.GetWidth();
            camera.screenSize.y() = resizeEvent.GetHeight();
            camera.target.x() = resizeEvent.GetWidth() / 2.0;
            camera.target.y() = resizeEvent.GetHeight() / 2.0;
            {
                auto texture = DeviceTexture::CreateForColorAttachment(m_Window->GetSize().x() * 4,
                                                               m_Window->GetSize().y() * 4,
                                                               PixelFormat::RGBA8888);
                m_OversamplingTexture = CreateScope<DeviceTexture>(std::move(texture.value()));
                m_TgOversamplingTexture->GetDesc().width= m_Window->GetSize().x() * 4;
                m_TgOversamplingTexture->GetDesc().height = m_Window->GetSize().y() * 4;
                m_TgOversamplingTexture->SetActualBorrow(m_OversamplingTexture.get());
                m_TgFinalTexture->GetDesc().width = m_Window->GetSize().x();
                m_TgFinalTexture->GetDesc().height = m_Window->GetSize().y();
                m_TgFinalTexture->SetActualBorrow(m_Window->GetFinalTexture());
            }
        }
    }
    virtual void OnRender(
        vk::RenderPass& renderPass,
        vk::FrameBuffer& framebuffer,
        vk::GraphicsCommandBuffer& commandBuffer) override
    {
        m_TaskGraph.SetCommandBuffer(commandBuffer);
        m_TaskGraph.Execute();
    }
    virtual void OnAttach(Window* window) override
    {
        m_Window = window;
        {
            auto downsampling = DownsamplingRenderPass::Create(ApplicationResource::GetSingleton().descriptorPool);
            if (!downsampling)
            {
                assert(false && "create downsampling render pass failed");
                return;
            }
            m_Downsampling = CreateScope<DownsamplingRenderPass>(std::move(downsampling.value()));
        }
        {
            auto texture = DeviceTexture::CreateForColorAttachment(m_Window->GetSize().x() * 4,
                                                                   m_Window->GetSize().y() * 4,
                                                                   PixelFormat::RGBA8888);
            m_OversamplingTexture = CreateScope<DeviceTexture>(std::move(texture.value()));
        }
        m_TgOversamplingTexture = m_TaskGraph.AddRetainedTextureBorrow("oversampling texture",
                                                                         m_OversamplingTexture.get(),
                                                                         DeviceImageLayout::Undefined);
        m_TgFinalTexture = m_TaskGraph.AddRetainedTextureBorrow("final image",
                                                                window->GetFinalTexture(),
                                                                DeviceImageLayout::ColorAttachment);

        DrawText(m_TgOversamplingTexture);
        //DrawText(m_TgFinalTexture);
        Downsampling(m_TgFinalTexture, m_TgOversamplingTexture);
        m_TaskGraph.Compile();
    }
    void Downsampling(TaskGraph::Texture* finalTexture, TaskGraph::Texture* oversamplingTexture)
    {
        struct TaskData
        {
        };
        TaskGraph::RenderPassDesc renderPassDesc;
        renderPassDesc.colorAttachmentCount = 1;
        renderPassDesc.colorAttachment[0].texture = finalTexture;
        renderPassDesc.colorAttachment[0].loadOp = DeviceAttachmentLoadOp::Clear;
        renderPassDesc.colorAttachment[0].storeOp = DeviceAttachmentStoreOp::Store;
        renderPassDesc.clearColor = Vec4f{1.0, 1.0, 1.0, 1.0};
        m_TaskGraph.AddRenderTask<TaskData>(renderPassDesc, [&](TaskGraph::RenderTaskBuilder& builder, TaskData&)
         { builder.Read(oversamplingTexture); }, 
         [this,oversamplingTexture](TaskData&, DeviceCommandBufferView& commandBuffer)
          { 
            m_Downsampling->Render(commandBuffer, *oversamplingTexture->GetActual(),ApplicationResource::GetSingleton().descriptorPool);
             });
    }
    void DrawText(TaskGraph::Texture* oversamplingTexture)
    {
        {
            struct TaskData
            {
            };
            TaskGraph::RenderPassDesc renderPassDesc;
            renderPassDesc.colorAttachmentCount = 1;
            renderPassDesc.colorAttachment[0].texture = oversamplingTexture;
            renderPassDesc.colorAttachment[0].loadOp = DeviceAttachmentLoadOp::Clear;
            renderPassDesc.colorAttachment[0].storeOp = DeviceAttachmentStoreOp::Store;
            renderPassDesc.clearColor = Vec4f{1.0, 1.0, 1.0, 1.0};
            m_TaskGraph.AddRenderTask<TaskData>(renderPassDesc, [&](TaskGraph::RenderTaskBuilder& builder, TaskData&) {

            },
                                                [this](TaskData&, DeviceCommandBufferView& commandBuffer) {
float framePerSec = m_Frame / m_Sec;
        std::string s = std::string("") + "fps: " + std::to_string(framePerSec) + "\n"
                        + "我能吞下玻璃而不伤身体\n"
                        + "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ\n"
                        + "Innovation in China 中国智造，慧及全球 0123456789\n"

            ;
        // get glyph curve
        ApplicationResource::GetSingleton().font->prepareGlyphsForText(s);

        U32String u32s(s);
        std::vector<uint32_t> unicodes;
        for (size_t i = 0; i < u32s.Size(); i++)
        {
            unicodes.push_back(u32s[i]);
        }
        // calculate glyph layout
        std::vector<Vec2f> glyphPos;
        glyphPos.reserve(unicodes.size());
        float worldSize = 16.0f;
        {
            float scale = worldSize / ApplicationResource::GetSingleton().font->emSize;
            float x = 0;
            float y = 0;
            float width = 700;
            float emSize = ApplicationResource::GetSingleton().font->emSize;
            for (auto& unicode : unicodes)
            {
                auto& glyph = ApplicationResource::GetSingleton().font->glyphs[unicode];
                if (unicode == '\n')
                {
                    x = 0;
                    y += worldSize;
                    glyphPos.push_back(Vec2f(0, 0));
                    continue;
                }
                glyphPos.push_back(Vec2f(x, y + (emSize - glyph.bearingY) * scale));
                x += (glyph.advance + glyph.kerningX) * scale;
                if (x > width)
                {
                    x = 0;
                    y += worldSize;
                }
            }
        }
        auto& camera = ApplicationResource::GetSingleton().camera;
        camera.CalculateMatrix();
        Text::Raster::RenderPassParam param{
            .commandBuffer = commandBuffer,
            .descriptorPool = ApplicationResource::GetSingleton().descriptorPool,
            .font = *ApplicationResource::GetSingleton().font,
            .unicodes = unicodes,
            .glyphPosition = glyphPos,
            .worldSize = worldSize,
            .camera = camera,
            .z = 0.0f};
        param.color = Vec3f(0, 0, 0);
        ApplicationResource::GetSingleton().textRaster->Render(param,
                                                               *ApplicationResource::GetSingleton().rasterResource); });
        }
    }
    virtual void OnUpdate(float sec) override
    {
        m_Sec += sec;
        m_Frame++;
    }

private:
    Window* m_Window;
    float m_Sec = 0;
    uint32_t m_Frame = 0;
    Scope<DeviceTexture> m_OversamplingTexture;
    TaskGraph::TaskGraph m_TaskGraph;
    Scope<DownsamplingRenderPass> m_Downsampling;
    TaskGraph::Texture* m_TgFinalTexture=nullptr;
    TaskGraph::Texture* m_TgOversamplingTexture = nullptr;
};