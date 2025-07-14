#pragma once
#include <Window/Layer.h>
#include <Render/TaskGraph.h>
#include <Window/Window.h>
#include "GlobalApplicationResource.h"
using namespace Aether;

class TestTaskGraphCompile : public Layer
{
public:
    virtual void OnAttach(Window* window) override
    {
        InitRenderResource(window);
        CreateTaskGraph();
    }

private:
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
};