#pragma once
#include "Render/RenderApi/DeviceFrameBuffer.h"
#include "Render/RenderApi/DeviceRenderPass.h"
#include "Render/RenderApi/DeviceTexture.h"
#include "Render/Vulkan/DescriptorSet.h"
#include "Render/Vulkan/GraphicsCommandPool.h"
#include "Render/Vulkan/Pipeline.h"
#include "Render/Vulkan/PipelineLayout.h"
#include "Render/Vulkan/RenderContext.h"
#include "Render/Vulkan/ShaderModule.h"
#include "UI/DynamicStagingBuffer.h"
#include "UI/Quad.h"
#include "Window/Layer.h"
#include "Window/Window.h"
#include "Window/WindowContext.h"
#include "Render/Vulkan/GlobalRenderContext.h"
#include "Render/Mesh/VkMesh.h"
#include "Render/Mesh/Geometry.h"
#include "Render/Shader/Compiler.h"
#include "Render/Utils.h"
#include <print>
#include "UI/Renderer.h"
#include "UI/Filter/Gamma.h"
using namespace Aether;
class TestLayer : public Layer
{
public:
    virtual void OnRender(
        vk::RenderPass& renderPass,
        vk::FrameBuffer& framebuffer,
        vk::GraphicsCommandBuffer& commandBuffer) override
    {
        m_RenderResource.m_DescriptorPool->Clear();
        m_FinalTexture.AsyncTransitionLayout(DeviceImageLayout::Texture, DeviceImageLayout::ColorAttachment, commandBuffer);
        commandBuffer.BeginRenderPass(m_RenderPass.GetVk(), m_FinalFrameBuffer.GetVk(),{0,0,0,1.0});
        // render quads to final image
        m_Renderer->Begin(m_RenderPass, m_FinalFrameBuffer, Vec2f(800, 600));
        for (auto& quad : m_Quads)
        {
            m_Renderer->DrawQuad(quad);
        }
        m_Renderer->End(commandBuffer);
        commandBuffer.EndRenderPass();
        m_FinalTexture.AsyncTransitionLayout(DeviceImageLayout::ColorAttachment, DeviceImageLayout::Texture, commandBuffer);
        commandBuffer.BeginRenderPass(renderPass, framebuffer, {0, 0, 0, 1.0});
        // gamma filter to screen
        m_GammaFilter->Render(m_FinalTexture, framebuffer, commandBuffer);
        commandBuffer.EndRenderPass();
    }
    virtual void OnAttach(Window* window) override
    {
        // create render resource
        CreateRenderResource();
        // create render pass
        assert(CreateRenderPass()&&"failed to create render pass for final image");
        //  create renderer
        auto renderer = UI::Renderer::Create(m_RenderPass,m_RenderResource);
        if (!renderer)
        {
            std::cout << renderer.error() << std::endl;
            return;
        }

        m_Renderer = CreateScope<UI::Renderer>(std::move(renderer.value()));
        m_Renderer->SetScreenSize(Vec2f(800, 600));
        // create quads
        UI::QuadDesc desc;
        desc.color = Vec4f(1, 0, 0, 1);
        desc.position = Vec2f(100, 100);
        desc.size = Vec2f(100, 100);
        m_Quads.push_back(UI::Quad(desc));
        desc.position.y() += 110;
        desc.color = Vec4f(0, 1, 0, 1);
        m_Quads.push_back(UI::Quad(desc));
        desc.position.x() += 110;
        desc.color = Vec4f(0, 0, 1, 1);
        m_Quads.push_back(UI::Quad(desc));
        desc.position.y() -= 110;
        desc.color = Vec4f(0.8392, 0.1215, 0.8549, 1);
        m_Quads.push_back(UI::Quad(desc));
        
        // create final texture
        m_FinalTexture = DeviceTexture::CreateForColorAttachment(window->GetSize().x(),
                                                         window->GetSize().y(),
                                                         PixelFormat::RGBA8888).value();
        // create final frame buffer
        m_FinalFrameBuffer=DeviceFrameBuffer::CreateFromTexture(m_RenderPass, m_FinalTexture);
        m_FinalTexture.SyncTransitionLayout(DeviceImageLayout::Undefined, DeviceImageLayout::Texture);
        // create gamma filter
        auto gammaFilter=UI::GammaFilter::Create(window->GetRenderPass(0), m_RenderResource.m_DescriptorPool.get());
        assert(gammaFilter&&"failed to create gamma filter");
        m_GammaFilter=CreateScope<UI::GammaFilter>(std::move(gammaFilter.value()));
        m_GammaFilter->SetGamma(2.2f);
    }
    bool CreateRenderPass();
    bool CreateRenderResource()
    {
        // descriptor pool
        auto descriptorPool=DeviceDescriptorPool::Create();
        if(!descriptorPool)
        {
            return false;
        }
        m_RenderResource.m_DescriptorPool=CreateScope<DeviceDescriptorPool>(std::move(descriptorPool.value()));
        // staging buffer
        auto stagingBuffer=UI::DynamicStagingBuffer(1024);
        m_RenderResource.m_StagingBuffer=CreateScope<UI::DynamicStagingBuffer>(std::move(stagingBuffer));
        return true;
    }
private:
    Scope<UI::Renderer> m_Renderer;
    std::vector<UI::Quad> m_Quads;
    DeviceTexture m_FinalTexture;
    DeviceFrameBuffer m_FinalFrameBuffer;
    DeviceRenderPass m_RenderPass;
    Scope<UI::GammaFilter> m_GammaFilter;
    UI::RenderResource m_RenderResource;
};
