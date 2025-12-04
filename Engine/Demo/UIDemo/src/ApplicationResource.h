#pragma once
#include "Render/PixelFormat.h"
#include "Render/RenderApi/DeviceFrameBuffer.h"
#include "Render/RenderApi/DeviceTexture.h"
#include "Render/Scene/Camera2D.h"
#include <UI/Render/Renderer.h>
using namespace Aether;
class ApplicationResource
{
public:
    Scope<UI::Renderer> renderer;
    UI::RenderResource renderResource;
    DeviceRenderPass renderPass;
    DeviceTexture* finalTexture; // window final image, not own by this class
    // color attachment: window final image
    // depath attachmetn: depthTexture
    DeviceFrameBuffer frameBuffer;
    DeviceTexture depthTexture;
    Camera2D camera;
    static std::optional<std::string> Init(const Vec2i& screenSize, DeviceTexture& _finalTexture)
    {
        s_Instance = new ApplicationResource();
        s_Instance->finalTexture = &_finalTexture;
        s_Instance->renderPass = vk::RenderPass::CreateForDepthTest().value();
        s_Instance->depthTexture = DeviceTexture::CreateForDepthAttachment(screenSize.x(),
                                                                           screenSize.y(), PixelFormat::R_FLOAT32_DEPTH)
                                       .value();
        s_Instance->depthTexture.SyncTransitionLayout(DeviceImageLayout::Undefined,
                                                      DeviceImageLayout::DepthStencilAttachment);

        s_Instance->frameBuffer = DeviceFrameBuffer::CreateFromColorAttachmentAndDepthAttachment(
            s_Instance->renderPass, *s_Instance->finalTexture, s_Instance->depthTexture);
        s_Instance->CreateRenderResource();
        s_Instance->CreateRenderer();
        s_Instance->InitCamera(screenSize.cast<float>());
        return std::nullopt;
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
            s_Instance->renderPass, *s_Instance->finalTexture, s_Instance->depthTexture);
        return true;
    }
    static void Destroy()
    {
        delete s_Instance;
        s_Instance = nullptr;
    }
    ~ApplicationResource()
    {
        if (renderer)
        {
            renderer.reset();
        }
        renderResource.m_DescriptorPool.reset();
        renderResource.m_StagingBuffer.reset();
        renderPass = DeviceRenderPass();
    }
    static ApplicationResource* s_Instance;

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
