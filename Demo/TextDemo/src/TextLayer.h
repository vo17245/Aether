#pragma once
#include <Window/Layer.h>
#include <Window/Window.h>
#include "ApplicationResource.h"
#include "Text/Raster/Raster.h"
#include "Window/WindowEvent.h"
#include <Core/String.h>
#include <variant>
#include "DownsamplingRenderPass.h"
#include <Render/RenderGraph/RenderGraph.h>
#ifdef DrawText
#undef DrawText
#endif
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
        }
    }
    virtual void RegisterRenderPasses(RenderGraph::RenderGraph& renderGraph)override
    {
        auto finalImage = m_Window->GetFinalImageAccessId();
        // draw text to final image
        struct TaskData
        {
        };
        renderGraph.AddRenderTask<TaskData>(
            [&](RenderGraph::RenderTaskBuilder& builder, TaskData& data) {
            },
            [](DeviceCommandBuffer& commandBuffer, RenderGraph::ResourceAccessor& accessor, TaskData& data) {

            });
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
                                                                   m_Window->GetSize().y() * 4, PixelFormat::RGBA8888);
            m_OversamplingTexture = CreateScope<DeviceTexture>(std::move(texture.value()));
        }
    }

    void DrawText(RenderGraph::AccessId<DeviceTexture> finalImage,RenderGraph::RenderGraph& renderGraph)
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
            m_TaskGraph.AddRenderTask<TaskData>(
                renderPassDesc,
                [&](TaskGraph::RenderTaskBuilder& builder, TaskData&) {

                },
                [this](TaskData&, DeviceCommandBufferView& commandBuffer) {
                    float framePerSec = m_Frame / m_Sec;
                    std::string s = std::string("") + "fps: " + std::to_string(framePerSec) + "\n"
                                    + "我能吞下玻璃而不伤身体\n"
                                    + "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ\n";

                    // get glyph curve
                    Text::StrokeParam strokeParam;
                    strokeParam.radius = 32;
                    auto lines = ApplicationResource::GetSingleton().font->prepareGlyphsForText(s, {});

                    size_t glyphCount = 0;
                    for (auto& line : lines)
                    {
                        glyphCount += line.visualGlyphs.size();
                    }
                    // calculate glyph layout
                    std::vector<Vec2f> glyphPos;
                    std::vector<uint32_t> glyphToRender;
                    glyphPos.reserve(glyphCount);
                    float worldSize = 32.0f;
                    {
                        float x = 0;
                        float y = 0;
                        float width = 700;

                        for (auto& line : lines)
                        {
                            for (auto& glyph : line.visualGlyphs)
                            {
                                auto& info =
                                    ApplicationResource::GetSingleton().font->bufferGlyphInfo[glyph.indexInBuffer];
                                float emSize = info.emSize;
                                float scale = worldSize / emSize;

                                glyphToRender.push_back(glyph.indexInBuffer);
                                glyphPos.push_back(Vec2f(x + (glyph.xOffset + info.bearingX) * scale,
                                                         y + (emSize - info.bearingY - glyph.yOffset) * scale));
                                x += glyph.xAdvance * scale;
                                y += glyph.yAdvance * scale;
                                if (x > width)
                                {
                                    x = 0;
                                    y += worldSize;
                                }
                            }
                            x = 0;
                            y += worldSize;
                        }
                    }
                    if (glyphToRender.empty())
                    {
                        return;
                    }
                    auto& camera = ApplicationResource::GetSingleton().camera;
                    camera.CalculateMatrix();
                    Text::Raster::RenderPassParam param{.commandBuffer = commandBuffer,
                                                        .descriptorPool =
                                                            ApplicationResource::GetSingleton().descriptorPool,
                                                        .font = *ApplicationResource::GetSingleton().font,
                                                        .bufferGlyphInfoIndexes = glyphToRender,
                                                        .glyphPosition = glyphPos,
                                                        .worldSize = worldSize,
                                                        .camera = camera,
                                                        .z = 0.0f};
                    param.color = Vec3f(0, 0, 0);

                    ApplicationResource::GetSingleton().textRaster->Render(
                        param, *ApplicationResource::GetSingleton().rasterResource);
                });
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
    Scope<DownsamplingRenderPass> m_Downsampling;
};