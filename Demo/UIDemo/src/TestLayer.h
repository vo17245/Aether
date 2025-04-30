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
#include "UI/Render/DynamicStagingBuffer.h"
#include "UI/Render/Quad.h"
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
#include "UI/Render/Renderer.h"
#include "UI/Render/Filter/Gamma.h"
#include "UI/Render/LoadTextureToLinear.h"
#include "Resource/Finder.h"
#include "Window/WindowEvent.h"
#include "ApplicationResource.h"
using namespace Aether;
class TestLayer : public Layer
{
public:
    virtual void OnRender(
        vk::RenderPass& renderPass,
        vk::FrameBuffer& framebuffer,
        vk::GraphicsCommandBuffer& commandBuffer) override
    {
        auto& defaultRenderPass= ApplicationResource::s_Instance->renderPass;
        auto& renderResource=ApplicationResource::s_Instance->renderResource;
        auto& renderer=*ApplicationResource::s_Instance->renderer;
        m_FinalTexture.AsyncTransitionLayout(DeviceImageLayout::Texture, DeviceImageLayout::ColorAttachment, commandBuffer);
        commandBuffer.BeginRenderPass(defaultRenderPass.GetVk(), m_FinalFrameBuffer.GetVk(), {0, 0, 0, 1.0});
        // render quads to final image
        renderer.Begin(renderPass, m_FinalFrameBuffer, m_WindowSize);
        for (auto& quad : m_Quads)
        {
            renderer.DrawQuad(quad);
        }
        renderer.End(commandBuffer);
        commandBuffer.EndRenderPass();
        m_FinalTexture.AsyncTransitionLayout(DeviceImageLayout::ColorAttachment, DeviceImageLayout::Texture, commandBuffer);
        commandBuffer.BeginRenderPass(renderPass, framebuffer, {0, 0, 0, 1.0});
        // gamma filter to screen
        m_GammaFilter->Render(m_FinalTexture, framebuffer, commandBuffer);
        commandBuffer.EndRenderPass();
    }
    virtual void OnAttach(Window* window) override
    {
        m_Finder.AddBundleDir("Assets/bundle");
        auto& renderResource = ApplicationResource::s_Instance->renderResource;
        auto& renderer = *ApplicationResource::s_Instance->renderer;
        auto& renderPass = ApplicationResource::s_Instance->renderPass;
        // load texture
        auto image = m_Finder.Find("Images/tiles.png");
        assert(image);
        assert(image->info);
        assert(image->info->type == Resource::AssetType::Image);
        auto& imageInfo = *(Resource::ImageInfo*)image->info.get();
        m_Texture = CreateRef<DeviceTexture>(UI::LoadTextureToLinear(image->path, imageInfo, *renderResource.m_StagingBuffer).value());
        // create basic quads
        CreateQuad({window->GetSize().x(), window->GetSize().y()});
        // create final texture
        auto textureEx = DeviceTexture::CreateForColorAttachment(window->GetSize().x(),
                                                                 window->GetSize().y(),
                                                                 PixelFormat::RGBA8888);
        assert(textureEx);
        m_FinalTexture = std::move(textureEx.value());
        // create final frame buffer
        m_FinalFrameBuffer = DeviceFrameBuffer::CreateFromTexture(renderPass, m_FinalTexture);
        m_FinalTexture.SyncTransitionLayout(DeviceImageLayout::Undefined, DeviceImageLayout::Texture);
        // create gamma filter
        auto gammaFilter = UI::GammaFilter::Create(window->GetRenderPass(0), renderResource.m_DescriptorPool.get());
        assert(gammaFilter && "failed to create gamma filter");
        m_GammaFilter = CreateScope<UI::GammaFilter>(std::move(gammaFilter.value()));
        m_GammaFilter->SetGamma(2.2f);
        m_WindowSize.x()=window->GetSize().x();
        m_WindowSize.y()=window->GetSize().y();
    }

    virtual void OnEvent(Event& event) override
    {
        auto& renderPass= ApplicationResource::s_Instance->renderPass;
        auto& renderer= *ApplicationResource::s_Instance->renderer;
        if (std::holds_alternative<WindowResizeEvent>(event))
        {
            auto& e = std::get<WindowResizeEvent>(event);
            CreateQuad({e.GetWidth(), e.GetHeight()});
            m_WindowSize = {e.GetWidth(), e.GetHeight()};
            if(m_WindowSize.x() == 0 || m_WindowSize.y() == 0)
            {
                return;
            }
            renderer.GetCamera().screenSize.x() = e.GetWidth();
            renderer.GetCamera().screenSize.y() = e.GetHeight();
            // create final texture
            
            auto textureEx = DeviceTexture::CreateForColorAttachment(m_WindowSize.x(),
                                                                     m_WindowSize.y(),
                                                                     PixelFormat::RGBA8888);
            assert(textureEx);
            m_FinalTexture = std::move(textureEx.value());
            // create final frame buffer
            m_FinalFrameBuffer = DeviceFrameBuffer::CreateFromTexture(renderPass, m_FinalTexture);
            m_FinalTexture.SyncTransitionLayout(DeviceImageLayout::Undefined, DeviceImageLayout::Texture);
        }
    }

private:
    void CreateQuad(Vec2f windowSize)
    {
        m_Quads.clear();
        float quadSize = std::min(windowSize.x(), windowSize.y()) * 0.3;
        // create basic quads
        UI::QuadDesc desc;
        desc.color = Vec4f(1, 0, 0, 1);
        desc.position = Vec3f(10, 10,0);
        desc.size = Vec2f(quadSize, quadSize);
        m_Quads.push_back(UI::Quad(desc));
        desc.position.y() += quadSize + 10;
        desc.color = Vec4f(0, 1, 0, 1);
        m_Quads.push_back(UI::Quad(desc));
        desc.position.x() += quadSize + 10;
        desc.color = Vec4f(0, 0, 1, 1);
        m_Quads.push_back(UI::Quad(desc));
        desc.position.y() -= quadSize + 10;
        desc.color = Vec4f(0.8392, 0.1215, 0.8549, 1);
        m_Quads.push_back(UI::Quad(desc));
        // create quad with texture
        desc.position.x() += quadSize + 10;
        auto quad = UI::Quad(desc);
        quad.SetTexture(m_Texture);
        m_Quads.push_back(std::move(quad));
    }

private:
    std::vector<UI::Quad> m_Quads;
    DeviceTexture m_FinalTexture;
    DeviceFrameBuffer m_FinalFrameBuffer;
    Scope<UI::GammaFilter> m_GammaFilter;
    Resource::Finder m_Finder;
    // texture
    Ref<DeviceTexture> m_Texture;
    Vec2f m_WindowSize;
};
