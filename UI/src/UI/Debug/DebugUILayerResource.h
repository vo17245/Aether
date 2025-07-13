#pragma once
#include "Render/PixelFormat.h"
#include "Render/RenderApi/DeviceFrameBuffer.h"
#include "Render/RenderApi/DeviceTexture.h"
#include "Render/Scene/Camera2D.h"
#include <UI/Render/Renderer.h>
#include <Core/Core.h>
namespace Aether::UI
{

class DebugUILayerResource
{
public:
    DebugUILayerResource(DebugUILayerResource&&) = default;
    DebugUILayerResource() = default;
    DebugUILayerResource(const DebugUILayerResource&) = delete;
    DebugUILayerResource& operator=(const DebugUILayerResource&) = delete;
    DebugUILayerResource& operator=(DebugUILayerResource&&) = default;

    Scope<UI::Renderer> renderer;
    UI::RenderResource renderResource;
    DeviceRenderPass renderPass;
    DeviceTexture* finalTexture; // window final image, not own by this class
    // color attachment: window final image
    // depath attachmetn: depthTexture
    DeviceFrameBuffer frameBuffer;
    DeviceTexture depthTexture;
    Camera2D camera;
    static std::optional<DebugUILayerResource> Create(const Vec2i& screenSize, DeviceTexture& _finalTexture)
    {
        auto res = DebugUILayerResource();
        res.finalTexture = &_finalTexture;
        res.renderPass = vk::RenderPass::CreateForDepthTest().value();
        res.depthTexture = DeviceTexture::CreateForDepthAttachment(screenSize.x(),
                                                                   screenSize.y(), PixelFormat::R_FLOAT32_DEPTH)
                               .value();
        res.depthTexture.SyncTransitionLayout(DeviceImageLayout::Undefined,
                                              DeviceImageLayout::DepthStencilAttachment);

        res.frameBuffer = DeviceFrameBuffer::CreateFromColorAttachmentAndDepthAttachment(
            res.renderPass, *res.finalTexture, res.depthTexture);
        res.CreateRenderResource();
        res.CreateRenderer();
        res.InitCamera(screenSize.cast<float>());
        return res;
    }
    bool ResizeHierarchyFrameBuffer(const Vec2i& screenSize, DeviceTexture& _finalTexture)
    {
        finalTexture = &_finalTexture;
        renderPass = vk::RenderPass::CreateForDepthTest().value();
        depthTexture = DeviceTexture::CreateForDepthAttachment(screenSize.x(),
                                                               screenSize.y(), PixelFormat::R_FLOAT32_DEPTH)
                           .value();
        depthTexture.SyncTransitionLayout(DeviceImageLayout::Undefined,
                                          DeviceImageLayout::DepthStencilAttachment);

        frameBuffer = DeviceFrameBuffer::CreateFromColorAttachmentAndDepthAttachment(
            renderPass, *finalTexture, depthTexture);
        return true;
    }
    ~DebugUILayerResource()
    {
        if (renderer)
        {
            renderer.reset();
        }
        renderResource.m_DescriptorPool.reset();
        renderResource.m_StagingBuffer.reset();
        renderPass = DeviceRenderPass();
    }

private:
    bool CreateRenderResource()
    {
        // descriptor pool
        auto descriptorPool = DeviceDescriptorPool::Create();
        if (!descriptorPool)
        {
            return false;
        }
        renderResource.m_DescriptorPool = CreateRef<DeviceDescriptorPool>(std::move(descriptorPool.value()));
        // staging buffer
        auto stagingBuffer = UI::DynamicStagingBuffer(1024);
        renderResource.m_StagingBuffer = CreateRef<UI::DynamicStagingBuffer>(std::move(stagingBuffer));
        return true;
    }
    bool CreateRenderer()
    {
        auto _renderer = UI::Renderer::Create(renderPass, renderResource);
        if (!_renderer)
        {
            assert(false && "failed to create ui renderer");
            return false;
        }
        renderer = CreateScope<UI::Renderer>(std::move(_renderer.value()));
        return true;
    }
    void InitCamera(const Vec2f& screenSize)
    {
        camera.screenSize = screenSize;
        camera.target = Vec2f(screenSize.x() / 2, screenSize.y() / 2);
        camera.offset = Vec2f(0, 0);
        camera.near = 0.0f;
        camera.far = 10000.0f;
        camera.zoom = 1.0f;
        camera.rotation = 0.0f;
    }
};
} // namespace Aether::UI
