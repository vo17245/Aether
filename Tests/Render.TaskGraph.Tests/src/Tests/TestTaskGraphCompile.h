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
        vk::GraphicsCommandBuffer& commandBuffer) override
    {
        m_TaskGraph->SetCommandBuffer(commandBuffer);
        m_TaskGraph->Execute();
    }

private:
    void CreateQuads(const Vec2u& screenSize)
    {
        const char* vs = R"(
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
        const char* fs = R"(
        #version 450
layout(location=0)in vec2 v_UV;
layout(location=0)out vec4 FragColor;

void main()
{

    FragColor=vec4(0.4,0.5,0.6,0.5);

}
        )";
        auto quadEx = DrawQuad::Create(vs, fs, Mat2x3f::Identity(), PixelFormat::RGBA8888, true);
        assert(quadEx.has_value() && "failed to create draw quad");
        m_Quads.push_back(CreateScope<DrawQuad>(std::move(quadEx.value())));
        m_Quads.back()->SetScreenSize(screenSize);
    }
    void CreateTaskGraph()
    {
        struct TaskData
        {
            
        };
        TaskGraph::Texture* finalTexture = m_TaskGraph->AddRetainedResourceBorrow<DeviceTexture>("final image",
                                                                                                 &(m_Window->GetFinalTexture()),
                                                                                                 TaskGraph::TextureDesc{
                                                                                                    .usages=(uint32_t)DeviceImageUsage::ColorAttachment,
                                                                                                    .pixelFormat = PixelFormat::RGBA8888,
                                                                                                    .width = (uint32_t)m_Window->GetSize().x(),
                                                                                                    .height = (uint32_t)m_Window->GetSize().y()

                                                                                                });

        auto data = m_TaskGraph->AddRenderTask<TaskData>(
            {.colorAttachment = {
                 {finalTexture, DeviceAttachmentLoadOp::Clear, DeviceAttachmentStoreOp::Store}},
             .depthAttachment = std::nullopt,
             .colorAttachmentCount = 1,
             .clearColor = {0.0f, 0.0f, 0.0f, 1.0f}},
            [&](TaskGraph::RenderTaskBuilder& builder, TaskData& data) {
                
            },
            [=,this](TaskData& data, DeviceCommandBufferView& commandBuffer) {
                for(auto& quad : m_Quads)
                {
                    quad->Render(commandBuffer);
                }
            });
        m_TaskGraph->Compile();
    }
    void InitRenderResource(Window* window)
    {
        auto commandBuffer = GlobalApplicationResource::GetInstance().GetCommandBufferPool().AllocateCommandBuffer();
        assert(!commandBuffer.Empty() && "failed to allocate command buffer");
        m_TaskGraph = CreateScope<TaskGraph::TaskGraph>();
        auto& finalImage = window->GetFinalTexture();


    }

private:
    Scope<TaskGraph::TaskGraph> m_TaskGraph;
    Scope<DeviceTexture> m_FinalImageTexture;
    std::vector<Scope<DrawQuad>> m_Quads;
    Window* m_Window = nullptr;
};