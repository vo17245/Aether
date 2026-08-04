#pragma once
#include "Core/Math/Def.h"
#include "UI/Hierarchy/Component/Base.h"
#include "UI/Hierarchy/Component/Text.h"
#include "UI/Render/Quad.h"
#include "Window/Layer.h"
#include "Window/Window.h"
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
    virtual void OnBuildRenderGraph(RenderGraph::RenderGraph& renderGraph) override
    {
        if (!m_CurrentPage)
        {
            return;
        }
        RenderGraph::RenderPassDesc renderPassDesc{};
        m_CurrentPage->OnBuildRenderGraph(renderGraph, renderPassDesc, m_ScreenSize);
    }
    virtual void OnAttach(Window* window) override
    {
        m_Window = window;
        auto resource = DebugUILayerResource::Create(window->GetSize());
        if (!resource)
        {
            assert(false && "failed to init resource");
            return;
        }
        m_Resource = std::move(resource.value());
        m_TextureCache = CreateScope<UI::TextureCache>(m_Resource.renderResource.m_StagingBuffer.get());
        m_ScreenSize.x() = window->GetSize().x();
        m_ScreenSize.y() = window->GetSize().y();
        m_Scene = CreateScope<World>();
        auto page = DebugPage::Load(m_CurrentPagePath,
                                    m_Scene.get(),
                                    m_Resource.renderResource.m_StagingBuffer.get(),
                                    m_TextureCache.get(),
                                    m_Resource.renderer.get(),
                                    &m_Resource.camera,
                                    std::move(m_CurrentPageBehavior));
        assert(page.has_value() && "failed to load debug page");
        if (page)
        {
            auto pageScope = CreateScope<DebugPage>(std::move(page.value()));
            SetCurrentPage(m_CurrentPagePath, std::move(pageScope));
        }
    }
    virtual void OnEvent(Event& event) override
    {
        if (std::holds_alternative<WindowResizeEvent>(event))
        {
            auto& e = std::get<WindowResizeEvent>(event);
            if (e.GetHeight() != 0 && e.GetWidth() != 0)
            {
                m_ScreenSize.x() = e.GetWidth();
                m_ScreenSize.y() = e.GetHeight();
                m_Resource.ResizeHierarchyFrameBuffer(Vec2i(e.GetWidth(), e.GetHeight()));
            }
        }
        if (m_CurrentPage)
        {
            m_CurrentPage->OnEvent(event);
        }
    }
    virtual void OnFrameBegin() override
    {
        if (m_CurrentPage)
        {
            m_CurrentPage->OnFrameBegin();
        }
        if (m_Resource.renderer)
        {
            m_Resource.renderer->OnFrameBegin();
        }
    }
    virtual void OnUpdate(float deltaSec) override
    {
        if (m_CurrentPage)
        {
            m_CurrentPage->OnUpdate(deltaSec);
        }
    }

private:
    void SetCurrentPage(const std::string path, Scope<DebugPage>&& page)
    {
        m_CurrentPage = std::move(page);
        m_CurrentPagePath = path;
        if (m_Window)
        {
            Event e = WindowResizeEvent(m_Window->GetSize().x(), m_Window->GetSize().y());
            m_CurrentPage->OnEvent(e);
        }
    }

private:
    Scope<World> m_Scene;
    Vec2f m_ScreenSize;
    Scope<UI::TextureCache> m_TextureCache;
    DebugUILayerResource m_Resource;
    Window* m_Window = nullptr;
    Scope<DebugPage> m_CurrentPage;
    Scope<IDebugPageBehavior> m_CurrentPageBehavior;
    std::string m_CurrentPagePath;
};
} // namespace Aether::UI
