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

    }
    void InitRenderResource(Window* window)
    {
        auto commandBuffer=GlobalApplicationResource::GetInstance().GetCommandBufferPool().AllocateCommandBuffer();
        assert(!commandBuffer.Empty() && "failed to allocate command buffer");
        m_TaskGraph = CreateScope<TaskGraph::TaskGraph>(std::move(commandBuffer));
        auto& finalImage = window->GetFinalTexture();
        // wrap final texture to taskgraph resource
        {
            TaskGraph::TextureDesc desc{
                .usages = (uint32_t)DeviceImageUsage::ColorAttachment,
                .pixelFormat = PixelFormat::RGBA8888,
                .width = finalImage.GetWidth(),
                .height = finalImage.GetHeight(),

            };
            auto texture = CreateScope<TaskGraph::Texture>("final image", nullptr, desc);
            m_TaskGraph->AddRetainedResource(std::move(texture));
        }
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
    std::vector<Scope<DrawQuad>> m_Quads;
};