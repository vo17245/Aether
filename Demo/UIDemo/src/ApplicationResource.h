#pragma once
#include <UI/Render/Renderer.h>
using namespace Aether;
class ApplicationResource
{
public:
    Scope<UI::Renderer> renderer;
    UI::RenderResource renderResource;
    DeviceRenderPass renderPass;
    static std::optional<std::string> Init(const Vec2i& screenSize)
    {
        s_Instance = new ApplicationResource();
        s_Instance->renderPass=vk::RenderPass::CreateDefault().value();
        s_Instance->CreateRenderResource();
        s_Instance->CreateRenderer();
        s_Instance->renderer->GetCamera().screenSize.x()=screenSize.x();
        s_Instance->renderer->GetCamera().screenSize.y()=screenSize.y();
        return std::nullopt;
    }
    static void Destroy()
    {
        delete s_Instance;
        s_Instance = nullptr;
    }
    ~ApplicationResource() 
    {
        if(renderer)
        {
            renderer.reset();
        }
        renderResource.m_DescriptorPool.reset();
        renderResource.m_StagingBuffer.reset();
        renderPass=DeviceRenderPass();
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

};
