#pragma once
#include <UI/UI.h>
#include <sol/sol.hpp>
#include "UI/Render/DynamicStagingBuffer.h"
#include "UI/Render/TextureCache.h"

namespace Aether::UI
{
class DebugPage;
class IDebugPageBehavior
{
public:
    virtual ~IDebugPageBehavior() = default;
    virtual void OnInit(DebugPage& page) {}
    virtual void OnEvent(DebugPage& page, Event& event) {}
    virtual void OnUpdate(DebugPage& page, float deltaSec) {}
};
class DebugPage
{
public:
    DebugPage(const DebugPage&) = delete;
    DebugPage& operator=(const DebugPage&) = delete;
    DebugPage(DebugPage&&) = default;
    DebugPage& operator=(DebugPage&&) = default;
    DebugPage(Borrow<World> scene, Borrow<UI::DynamicStagingBuffer> stagingBuffer, Borrow<UI::TextureCache> textureCache) :
        m_Scene(scene), m_StagingBuffer(stagingBuffer), m_TextureCache(textureCache)
    {
    }
    void OnFrameBegin()
    {
        if (m_Hierarchy)
        {
            m_Hierarchy->OnFrameBegin();
        }
    }
    void OnUpdate(float deltaSec)
    {
        if (m_Behavior)
        {
            m_Behavior->OnUpdate(*this, deltaSec);
        }
        if (m_Hierarchy)
        {
            m_Hierarchy->OnUpdate(deltaSec);
        }
    }
    void OnBuildRenderGraph(RenderGraph::RenderGraph& renderGraph,
                            const RenderGraph::RenderPassDesc& renderPassDesc,
                            Vec2f screenSize)
    {
        if (m_Hierarchy)
        {
            m_Hierarchy->OnBuildRenderGraph(renderGraph, renderPassDesc, screenSize);
        }
    }
    void OnEvent(Event& event)
    {
        if (m_Behavior)
        {
            m_Behavior->OnEvent(*this, event);
        }
        if (m_Hierarchy)
        {
            m_Hierarchy->OnEvent(event);
        }
    }
    static std::expected<DebugPage, std::string> Load(const std::string& pagePath,
                                                      Borrow<World> scene,
                                                      Borrow<UI::DynamicStagingBuffer> stagingBuffer,
                                                      Borrow<UI::TextureCache> textureCache,
                                                      Borrow<UI::Renderer> renderer,
                                                      Borrow<Camera2D> camera,
                                                      Scope<IDebugPageBehavior>&& behavior = nullptr)
    {
        DebugPage page(scene, stagingBuffer, textureCache);
        page.m_Behavior = std::move(behavior);
        page.m_Hierarchy = std::make_unique<UI::Hierarchy>(page.m_Scene);
        auto error = page.InitHierarchy(pagePath, renderer, camera);
        if (error)
        {
            return std::unexpected(error.value());
        }
        page.OnInit();
        return page;
    }
    bool IsLoaded() const
    {
        return m_Hierarchy != nullptr && m_Scene != nullptr;
    }
    UI::Hierarchy& GetHierarchy()
    {
        return *m_Hierarchy;
    }

private:
    void OnInit()
    {
        if (m_Behavior)
        {
            m_Behavior->OnInit(*this);
        }
    }
    std::optional<std::string> InitHierarchy(const std::string& hierarchyLuaPath,
                                             Borrow<UI::Renderer> renderer,
                                             Borrow<Camera2D> camera)
    {
        UI::QuadSystem* quadSystem = new UI::QuadSystem(m_TextureCache);
        quadSystem->SetCamera(&m_Hierarchy->GetCamera());
        quadSystem->renderer = renderer.Get();
        m_Hierarchy->AddSystem(quadSystem);

        UI::TextSystem* textSystem = UI::TextSystem::Create();
        if (textSystem)
        {
            textSystem->SetCamera(&m_Hierarchy->GetCamera());
            textSystem->AddAssetDir("Assets");
            m_Hierarchy->AddSystem(textSystem);
        }

        UI::MouseSystem* mouseSystem = new UI::MouseSystem();
        m_Hierarchy->AddSystem(mouseSystem);
        UI::InputTextSystem* inputTextSystem = new UI::InputTextSystem();
        m_Hierarchy->AddSystem(inputTextSystem);
        UI::AutoResizeSystem* autoResizeSystem = new UI::AutoResizeSystem();
        m_Hierarchy->AddSystem(autoResizeSystem);
        UI::VisibilityRequestSystem* visibilityRequestSystem = new UI::VisibilityRequestSystem();
        m_Hierarchy->AddSystem(visibilityRequestSystem);

        UI::Lua::HierarchyLoader loader;
        loader.PushNodeCreator<UI::Lua::GridNodeCreator>("grid");
        loader.PushNodeCreator<UI::Lua::QuadNodeCreator>("quad");
        loader.PushNodeCreator<UI::Lua::TextNodeCreator>("text");
        auto hierarchyStrOpt = Filesystem::ReadFileToString(hierarchyLuaPath);
        if (!hierarchyStrOpt)
        {
            return std::format("Failed to read hierarchy file: {}", hierarchyLuaPath);
        }
        auto err = loader.LoadHierarchy(*m_Hierarchy, *hierarchyStrOpt);
        if (err)
        {
            return "Failed to load hierarchy: " + err.value();
        }
        autoResizeSystem->SetRoot(m_Hierarchy->GetRoot());
        Event e = WindowResizeEvent(800, 600);
        autoResizeSystem->OnEvent(e, m_Hierarchy->GetScene());
        (void)camera;
        return std::nullopt;
    }

private:
    Scope<UI::Hierarchy> m_Hierarchy;
    Borrow<World> m_Scene;
    Borrow<UI::DynamicStagingBuffer> m_StagingBuffer;
    Borrow<UI::TextureCache> m_TextureCache;
    Scope<IDebugPageBehavior> m_Behavior;
};
} // namespace Aether::UI
