#pragma once
#include "Core/Math/Def.h"
#include "Render/RenderApi.h"

#include "Render/Vulkan/RenderContext.h"
#include "UI/Hierarchy/Component/Base.h"
#include "UI/Hierarchy/Component/Text.h"
#include "UI/Render/Quad.h"
#include "Window/Layer.h"
#include "Window/Window.h"
#include "Window/WindowContext.h"
#include <print>
#include <variant>
#include "Window/WindowEvent.h"
#include <UI/Hierarchy/Hierarchy.h>
#include <UI/Hierarchy/System/Quad.h>
#include <UI/Hierarchy/Loader/HierarchyXmlLoader.h>
#include <UI/Hierarchy/Loader/NodeCreator.h>
#include <UI/Hierarchy/Loader/BuiltinXmlNodeCreator.h>
#include <UI/Hierarchy/System/Text.h>
#include <UI/Hierarchy/System/Mouse.h>
#include <UI/Hierarchy/System/InputText.h>
#include "DebugUILayerResource.h"
#include "DebugPage.h"

namespace Aether::UI
{

class DebugUILayer : public Layer
{
public:
    DebugUILayer(const std::string& currentPagePath, Scope<IDebugPageBehavior>&& behavior = nullptr) :
        m_CurrentPagePath(currentPagePath), m_CurrentPageBehavior(std::move(behavior))
    {
    }
    ~DebugUILayer()
    {
    }
    virtual void OnDetach() override
    {
    }
    virtual void OnRender(
        vk::RenderPass& renderPass,
        vk::FrameBuffer& framebuffer,
        vk::GraphicsCommandBuffer& commandBuffer) override
    {
        auto& renderer = *m_Resource.renderer;
        auto& camera = m_Resource.camera;
        camera.target = Vec2f(m_ScreenSize.x() / 2, m_ScreenSize.y() / 2);
        std::vector<VkClearValue> clearValues(2);
        clearValues[0].color = {0.0f, 0.0f, 0.0f, 1.0f};
        clearValues[1].depthStencil = {1.0f, 0};
        commandBuffer.BeginRenderPass(m_Resource.renderPass.GetVk(),
                                      m_Resource.frameBuffer.GetVk(),
                                      clearValues);
        m_CurrentPage->OnRender(commandBuffer, framebuffer,
                                m_ScreenSize);
        commandBuffer.EndRenderPass();
    }
    virtual void OnAttach(Window* window) override
    {
        m_Window = window;
        {
            auto resource = DebugUILayerResource::Create(window->GetSize(), window->GetFinalTexture());
            if (!resource)
            {
                assert(false && "failed to init resource");
                return;
            }
            m_Resource = std::move(resource.value());
        }
        m_TextureCache = CreateScope<UI::TextureCache>(
            m_Resource.renderResource.m_StagingBuffer.get());

        m_ScreenSize.x() = window->GetSize().x();
        m_ScreenSize.y() = window->GetSize().y();
        m_Scene = CreateScope<Scene>();
        // load current page
        {
            auto page = DebugPage::Load(m_CurrentPagePath, *m_Scene, *m_Resource.renderResource.m_StagingBuffer,
                                        *m_TextureCache, *m_Resource.renderer, *m_Resource.renderResource.m_DescriptorPool,
                                        m_Resource.renderPass, m_Resource.camera, std::move(m_CurrentPageBehavior));
            assert(page.has_value() && "failed to load debug page");
            auto pageScope = CreateScope<DebugPage>(std::move(page.value()));
            SetCurrentPage(m_CurrentPagePath, std::move(pageScope));
        }
    }

    virtual void OnEvent(Event& event) override
    {
        if (std::holds_alternative<WindowResizeEvent>(event))
        {
            auto& e = std::get<WindowResizeEvent>(event);
            if (e.GetHeight() == 0 || e.GetWidth() == 0)
            {
            }
            else
            {
                // update camera
                auto& camera = m_Resource.camera;

                auto& renderer = *m_Resource.renderer;
                camera.screenSize.x() = e.GetWidth();
                camera.screenSize.y() = e.GetHeight();
                // update hierarchy framebuffer
                m_Resource.ResizeHierarchyFrameBuffer(Vec2i(e.GetWidth(), e.GetHeight()),
                                                      m_Window->GetFinalTexture());
            }
        }
        m_CurrentPage->OnEvent(event);
    }
    virtual void OnFrameBegin() override
    {
        m_CurrentPage->OnFrameBegin();
        m_Resource.renderer->OnFrameBegin();
    }
    virtual void OnUpdate(float deltaSec) override
    {
        m_CurrentPage->OnUpdate(deltaSec);
    }

private:
    void SetCurrentPage(const std::string path, Scope<DebugPage>&& page)
    {
        m_CurrentPage = std::move(page);
        m_CurrentPagePath = path;
        Event e = WindowResizeEvent(m_Window->GetSize().x(), m_Window->GetSize().y());
        m_CurrentPage->OnEvent(e);
    }
    std::unique_ptr<Scene> m_Scene;
    Vec2f m_ScreenSize;
    std::unique_ptr<UI::TextureCache> m_TextureCache;
    DebugUILayerResource m_Resource;
    Window* m_Window = nullptr;
    Scope<DebugPage> m_CurrentPage;
    Scope<IDebugPageBehavior> m_CurrentPageBehavior; // set on construtor, move to page on attach, **will be null after OnAttach**
    std::string m_CurrentPagePath;
};
} // namespace Aether::UI
