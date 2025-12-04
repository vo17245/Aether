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
    virtual void  OnInit(DebugPage& page){}
    virtual void OnEvent(DebugPage& page,Event& event){}
    virtual void OnUpdate(DebugPage& page,float deltaSec){}
};
class DebugPage 
{
public:    
    ~DebugPage()
    {
        if (m_Hierarchy)
        {
            m_Hierarchy.reset();
        }
    }
    DebugPage(const DebugPage&) = delete;
    DebugPage& operator=(const DebugPage&) = delete;
    DebugPage(DebugPage&&) = default;
    DebugPage& operator=(DebugPage&&) = default;
    DebugPage(Borrow<Scene> scene,Borrow<UI::DynamicStagingBuffer> stagingBuffer,Borrow<UI::TextureCache> textureCache)
    :m_Scene(scene),m_TextureCache(textureCache){}
public:
    void OnFrameBegin()
    {
        m_Hierarchy->OnFrameBegin();
    }
    void OnUpdate(float deltaSec)
    {
        if(m_Behavior)
        {
            m_Behavior->OnUpdate(*this, deltaSec);
        }
        m_Hierarchy->OnUpdate(deltaSec);
    }
    
    void OnRender(DeviceCommandBufferView commandBuffer,
                  DeviceFrameBufferView frameBuffer,
                  Vec2f screenSize)
    {
        m_Hierarchy->OnRender(commandBuffer,  frameBuffer, screenSize);
    }
    void OnEvent(Event& event)
    {
        if(m_Behavior)
        {
            m_Behavior->OnEvent(*this, event);
        }
        m_Hierarchy->OnEvent(event);
    }
    static std::expected<DebugPage,std::string> Load(const std::string& pagePath,Borrow<Scene> scene,
    Borrow<UI::DynamicStagingBuffer> stagingBuffer,
    Borrow<UI::TextureCache> textureCache,
    Borrow<UI::Renderer> renderer,
    Borrow<DeviceDescriptorPool> descriptorPool,
    Borrow<DeviceRenderPass> renderPass,
    Borrow<Camera2D> camera,
    Scope<IDebugPageBehavior>&& behavior=nullptr)
    {
        DebugPage page(scene,stagingBuffer,textureCache);
        
        page.m_Behavior=std::move(behavior);
        
        // create hierarchy
        page.m_Hierarchy=std::make_unique<UI::Hierarchy>(page.m_Scene);
        auto error=page.InitHierarchy(pagePath,renderer,descriptorPool,renderPass,camera);
        if(error)
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
    std::optional<std::string> InitHierarchy(const std::string& hierarchyLuaPath,Borrow<UI::Renderer> renderer,
    Borrow<DeviceDescriptorPool> descriptorPool,Borrow<DeviceRenderPass> renderPass,Borrow<Camera2D> _camera)
    {
        // quad
        UI::QuadSystem* quadSystem = new UI::QuadSystem(m_TextureCache);
        quadSystem->SetCamera(&m_Hierarchy->GetCamera());
        quadSystem->renderer = renderer.Get();
        m_Hierarchy->AddSystem(quadSystem);
        // text
        UI::TextSystem* textSystem = UI::TextSystem::Create(*renderPass,
                                                            *descriptorPool);
        textSystem->SetDescriptorPool(descriptorPool.Get());
        textSystem->SetCamera(&m_Hierarchy->GetCamera());
        textSystem->AddAssetDir("Assets");
        m_Hierarchy->AddSystem(textSystem);
        // mouse
        UI::MouseSystem* mouseSystem = new UI::MouseSystem();
        m_Hierarchy->AddSystem(mouseSystem);
        // input text
        UI::InputTextSystem* inputTextSystem = new UI::InputTextSystem();
        m_Hierarchy->AddSystem(inputTextSystem);
        // auto resize
        UI::AutoResizeSystem* autoResizeSystem=new UI::AutoResizeSystem();
        m_Hierarchy->AddSystem(autoResizeSystem);
        // visibility request
        UI::VisibilityRequestSystem* visibilityRequestSystem = new UI::VisibilityRequestSystem();
        m_Hierarchy->AddSystem(visibilityRequestSystem);


        UI::Lua::HierarchyLoader loader;
        loader.PushNodeCreator<UI::Lua::GridNodeCreator>("grid");
        loader.PushNodeCreator<UI::Lua::QuadNodeCreator>("quad");
        loader.PushNodeCreator<UI::Lua::TextNodeCreator>("text");
        auto hierarchyStrOpt=Filesystem::ReadFileToString(hierarchyLuaPath);
        if(!hierarchyStrOpt)
        {
            return std::format("Failed to read hierarchy file: {}", hierarchyLuaPath) ;
        }
        auto& hierarchyStrValue = *hierarchyStrOpt;

        auto err=loader.LoadHierarchy(*m_Hierarchy, hierarchyStrValue);
        if(err)
        {
            return "Failed to load hierarchy: " + err.value();
        }
        autoResizeSystem->SetRoot(m_Hierarchy->GetRoot());
        {
            Event e=WindowResizeEvent(800,600);
            autoResizeSystem->OnEvent(e, m_Hierarchy->GetScene());
        }
        auto& camera = *_camera;
        return std::nullopt;
    }
    Scope<UI::Hierarchy> m_Hierarchy;//Hierarchy means ui system in Aether
    Borrow<Scene> m_Scene;
    Borrow<UI::TextureCache> m_TextureCache;
    Scope<IDebugPageBehavior> m_Behavior;

};
}
