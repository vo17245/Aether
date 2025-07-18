#pragma once
#include <Window/Layer.h>
#include <Render/TaskGraph.h>
#include <Window/Window.h>
#include "GlobalApplicationResource.h"
#include "DrawQuad.h"
#include "Test.h"
using namespace Aether;

class TestTaskGraphCompile : public Test
{
public:
    virtual void OnAttach(Window* window) override
    {
        m_Window = window;
        InitRenderResource(window);
        CreateQuads(window->GetSize().cast<uint32_t>());
        CreateTaskGraph();
    }
virtual void OnRender(
        vk::RenderPass& renderPass,
        vk::FrameBuffer& framebuffer,
        vk::GraphicsCommandBuffer& commandBuffer
    )override
    {
        DeviceCommandBufferView cbv(commandBuffer);

        commandBuffer.BeginRenderPass(renderPass,
                                      framebuffer,
                                      Vec4(0.0,0.0,0.0,1.0));
        for(auto& quad:m_Quads)
        {
            quad->Render(cbv);
        }
        commandBuffer.EndRenderPass();
    }
private:
    void CreateQuads(const Vec2u& screenSize)
    {
        const char* vs=R"(
        #version 450
layout(location=0)in vec2 a_Position;
layout(location=1)in vec2 a_UV;

layout(location=0)out vec2 v_UV;
void main()
{
    gl_Position=vec4(a_Position,0.0,1.0);
    v_UV=a_UV;
}
        )";
        const char* fs=R"(
        #version 450
layout(location=0)in vec2 v_UV;
layout(location=0)out vec4 FragColor;

void main()
{

    FragColor=vec4(0.4,0.5,0.6,0.5);

}
        )";
        auto quadEx=DrawQuad::Create(vs, fs, Mat2x3f::Identity(), PixelFormat::RGBA8888, true);
        assert(quadEx.has_value() && "failed to create draw quad");
        m_Quads.push_back(CreateScope<DrawQuad>(std::move(quadEx.value())));
        m_Quads.back()->SetScreenSize(screenSize);
    }
    void CreateTaskGraph()
    {
        struct TaskData
        {
            TaskGraph::FrameBuffer* frameBuffer;
        };
        TaskGraph::Texture* finalTexture=m_TaskGraph->AddRetainedResourceBorrow<DeviceTexture>("final image",
            &(m_Window->GetFinalTexture()),
            TaskGraph::TextureDesc{});
        auto data=m_TaskGraph->AddRenderTask<TaskData>(
            [&](TaskGraph::TaskBuilder& builder,TaskData& data){
                data.frameBuffer=builder.Create<TaskGraph::FrameBuffer>("final texture framebuffer", TaskGraph::FrameBufferDesc{
                    .colorAttachments={finalTexture}
                });
                builder.Write(finalTexture);
            }, 
            [=](TaskData& data,DeviceCommandBuffer& commandBuffer){

            });
    }
    void InitRenderResource(Window* window)
    {
        auto commandBuffer=GlobalApplicationResource::GetInstance().GetCommandBufferPool().AllocateCommandBuffer();
        assert(!commandBuffer.Empty() && "failed to allocate command buffer");
        m_TaskGraph = CreateScope<TaskGraph::TaskGraph>(std::move(commandBuffer));
        auto& finalImage = window->GetFinalTexture();

        // create final image render pass
        {
            DeviceRenderPassDesc desc;
            desc.colorAttachmentCount = 1;
            auto& colorAttach = desc.colorAttachments[0];
            colorAttach.format = PixelFormat::RGBA8888;
            colorAttach.load = DeviceAttachmentLoadOp::Clear;
            colorAttach.store = DeviceAttachmentStoreOp::Store;
            m_FinalImageRenderPass = CreateScope<DeviceRenderPass>(DeviceRenderPass::Create(desc));
            assert(!m_FinalImageRenderPass->Empty() && "failed to create final image render pass");
        }
        // create final image framebuffer
        {
            auto frameBuffer = DeviceFrameBuffer::CreateFromColorAttachment(*m_FinalImageRenderPass, finalImage);
            assert(!frameBuffer.Empty());
            m_FinalImageFrameBuffer = CreateScope<DeviceFrameBuffer>(std::move(frameBuffer));
        }
    }

private:
    Scope<TaskGraph::TaskGraph> m_TaskGraph;
    Scope<DeviceFrameBuffer> m_FinalImageFrameBuffer;
    Scope<DeviceRenderPass> m_FinalImageRenderPass;
    Scope<DeviceTexture> m_FinalImageTexture;
    std::vector<Scope<DrawQuad>> m_Quads;
    Window* m_Window = nullptr;
};