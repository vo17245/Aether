#pragma once
#include "Core/Math/Def.h"
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
#include <variant>
#include "UI/Renderer.h"
#include "UI/Filter/Gamma.h"
#include "UI/LoadTextureToLinear.h"
#include "Resource/Finder.h"
#include "Window/WindowEvent.h"
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
        commandBuffer.BeginRenderPass(m_RenderPass.GetVk(), m_FinalFrameBuffer.GetVk(), {0, 0, 0, 1.0});
        // render quads to final image
        m_Renderer->Begin(m_RenderPass, m_FinalFrameBuffer, m_WindowSize);
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
        m_Finder.AddBundleDir("Assets");
        // create render resource
        CreateRenderResource();
        // create render pass
        assert(CreateRenderPass() && "failed to create render pass for final image");
        //  create renderer
        auto renderer = UI::Renderer::Create(m_RenderPass, m_RenderResource);
        if (!renderer)
        {
            assert(false && "failed to create ui renderer");
            return;
        }

        m_Renderer = CreateScope<UI::Renderer>(std::move(renderer.value()));
        m_WindowSize = {window->GetSize().x(), window->GetSize().y()};
        m_Renderer->SetScreenSize(m_WindowSize);
        // load texture
        auto image = m_Finder.Find("Images/tiles.png");
        assert(image);
        assert(image->info);
        assert(image->info->type == Resource::AssetType::Image);
        auto& imageInfo = *(Resource::ImageInfo*)image->info.get();
        m_Texture = CreateRef<DeviceTexture>(UI::LoadTextureToLinear(image->path, imageInfo, *m_RenderResource.m_StagingBuffer).value());
        // create basic quads
        CreateQuad({window->GetSize().x(), window->GetSize().y()});
        // create final texture
        auto textureEx = DeviceTexture::CreateForColorAttachment(window->GetSize().x(),
                                                                 window->GetSize().y(),
                                                                 PixelFormat::RGBA8888);
        assert(textureEx);
        m_FinalTexture = std::move(textureEx.value());
        // create final frame buffer
        m_FinalFrameBuffer = DeviceFrameBuffer::CreateFromTexture(m_RenderPass, m_FinalTexture);
        m_FinalTexture.SyncTransitionLayout(DeviceImageLayout::Undefined, DeviceImageLayout::Texture);
        // create gamma filter
        auto gammaFilter = UI::GammaFilter::Create(window->GetRenderPass(0), m_RenderResource.m_DescriptorPool.get());
        assert(gammaFilter && "failed to create gamma filter");
        m_GammaFilter = CreateScope<UI::GammaFilter>(std::move(gammaFilter.value()));
        m_GammaFilter->SetGamma(2.2f);
    }
    bool CreateRenderPass();
    bool CreateRenderResource()
    {
        // descriptor pool
        auto descriptorPool = DeviceDescriptorPool::Create();
        if (!descriptorPool)
        {
            return false;
        }
        m_RenderResource.m_DescriptorPool = CreateScope<DeviceDescriptorPool>(std::move(descriptorPool.value()));
        // staging buffer
        auto stagingBuffer = UI::DynamicStagingBuffer(1024);
        m_RenderResource.m_StagingBuffer = CreateScope<UI::DynamicStagingBuffer>(std::move(stagingBuffer));
        return true;
    }
    virtual void OnEvent(Event& event) override
    {
        if (std::holds_alternative<WindowResizeEvent>(event))
        {
            auto& e = std::get<WindowResizeEvent>(event);
            CreateQuad({e.GetWidth(), e.GetHeight()});
            m_WindowSize = {e.GetWidth(), e.GetHeight()};
            // create final texture
            auto textureEx = DeviceTexture::CreateForColorAttachment(m_WindowSize.x(),
                                                                     m_WindowSize.y(),
                                                                     PixelFormat::RGBA8888);
            assert(textureEx);
            m_FinalTexture = std::move(textureEx.value());
            // create final frame buffer
            m_FinalFrameBuffer = DeviceFrameBuffer::CreateFromTexture(m_RenderPass, m_FinalTexture);
            m_FinalTexture.SyncTransitionLayout(DeviceImageLayout::Undefined, DeviceImageLayout::Texture);
        }
    }

private:
    void CreateQuad(Vec2f windowSize)
    {
        m_Renderer->SetScreenSize(windowSize);
        m_Quads.clear();
        float quadSize = std::min(windowSize.x(), windowSize.y()) * 0.3;
        // create basic quads
        UI::QuadDesc desc;
        desc.color = Vec4f(1, 0, 0, 1);
        desc.position = Vec2f(10, 10);
        desc.size = Vec2f(quadSize, quadSize);
        m_Quads.push_back(UI::Quad(desc));
        desc.position.y() += quadSize + 10;
        ;
        desc.color = Vec4f(0, 1, 0, 1);
        m_Quads.push_back(UI::Quad(desc));
        desc.position.x() += quadSize + 10;
        ;
        desc.color = Vec4f(0, 0, 1, 1);
        m_Quads.push_back(UI::Quad(desc));
        desc.position.y() -= quadSize + 10;
        desc.color = Vec4f(0.8392, 0.1215, 0.8549, 1);
        m_Quads.push_back(UI::Quad(desc));
        // create quad with texture
        desc.position.x() += quadSize + 10;
        ;
        auto quad = UI::Quad(desc);
        quad.SetTexture(m_Texture);
        m_Quads.push_back(std::move(quad));
    }

private:
    Scope<UI::Renderer> m_Renderer;
    std::vector<UI::Quad> m_Quads;
    DeviceTexture m_FinalTexture;
    DeviceFrameBuffer m_FinalFrameBuffer;
    DeviceRenderPass m_RenderPass;
    Scope<UI::GammaFilter> m_GammaFilter;
    UI::RenderResource m_RenderResource;
    Resource::Finder m_Finder;
    // texture
    Ref<DeviceTexture> m_Texture;
    Vec2f m_WindowSize;
};
