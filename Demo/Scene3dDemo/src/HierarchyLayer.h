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
#include "UI/Hierarchy/Component/Base.h"
#include "UI/Hierarchy/Component/Text.h"
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
#include <UI/Hierarchy/Hierarchy.h>
#include <UI/Hierarchy/System/Quad.h>
#include <UI/Hierarchy/Loader/HierarchyLuaLoader.h>
#include <UI/Hierarchy/Loader/NodeCreator.h>
#include <UI/Hierarchy/Loader/BuiltinXmlNodeCreator.h>
#include <UI/Hierarchy/System/Text.h>
#include <UI/Hierarchy/System/Mouse.h>
#include <UI/Hierarchy/System/InputText.h>
#include <UI/Hierarchy/Loader/BuiltinLuaNodeCreator.h>
#include <UI/Hierarchy/System/AutoResize.h>
using namespace Aether;
class HierarchyLayer : public Layer
{
public:
    ~HierarchyLayer()
    {
    }
    virtual void OnRender(
        vk::RenderPass& renderPass,
        vk::FrameBuffer& framebuffer,
        vk::GraphicsCommandBuffer& commandBuffer) override
    {
        auto& renderer = *ApplicationResource::s_Instance->renderer;
        auto& camera = ApplicationResource::s_Instance->camera;
        camera.target = Vec2f(m_ScreenSize.x() / 2, m_ScreenSize.y() / 2);
        std::vector<VkClearValue> clearValues(2);
        clearValues[0].color = {0.0f, 0.0f, 0.0f, 1.0f};
        clearValues[1].depthStencil = {1.0f, 0};
        commandBuffer.BeginRenderPass(ApplicationResource::s_Instance->renderPass.GetVk(),
                                      ApplicationResource::s_Instance->frameBuffer.GetVk(),
                                      clearValues);
        m_Hierarchy.OnRender(commandBuffer, renderPass, framebuffer,
                             m_ScreenSize);
        commandBuffer.EndRenderPass();
    }
    virtual void OnAttach(Window* window) override
    {
        Debug::Log::Debug("HierarchyLayer Attach");
        m_ScreenSize.x() = window->GetSize().x();
        m_ScreenSize.y() = window->GetSize().y();
        // quad
        UI::QuadSystem* quadSystem = new UI::QuadSystem();
        quadSystem->SetCamera(&m_Hierarchy.GetCamera());
        quadSystem->renderer = ApplicationResource::s_Instance->renderer.get();
        m_Hierarchy.AddSystem(quadSystem);
        // text
        UI::TextSystem* textSystem = UI::TextSystem::Create(ApplicationResource::s_Instance->renderPass,
                                                            *ApplicationResource::s_Instance->renderResource.m_DescriptorPool,
                                                            window->GetSize().cast<float>());
        textSystem->SetDescriptorPool(ApplicationResource::s_Instance->renderResource.m_DescriptorPool.get());
        textSystem->SetCamera(&m_Hierarchy.GetCamera());
        textSystem->AddAssetDir("Assets");
        m_Hierarchy.AddSystem(textSystem);
        // mouse
        UI::MouseSystem* mouseSystem = new UI::MouseSystem();
        m_Hierarchy.AddSystem(mouseSystem);
        // input text
        UI::InputTextSystem* inputTextSystem = new UI::InputTextSystem();
        m_Hierarchy.AddSystem(inputTextSystem);
        // auto resize
        UI::AutoResizeSystem* autoResizeSystem=new UI::AutoResizeSystem();
        m_Hierarchy.AddSystem(autoResizeSystem);
        // load hierarchy
        InitHierarchyFromLua();
        autoResizeSystem->SetRoot(m_Hierarchy.GetRoot());
        {
            Event e=WindowResizeEvent(800,600);
            autoResizeSystem->OnEvent(e, m_Hierarchy.GetScene());
        }
        auto& camera = ApplicationResource::s_Instance->camera;
        m_Hierarchy.RebuildLayout(m_ScreenSize, camera.far);
    }
    void InitHierarchyFromLua()
    {
        auto& camera = ApplicationResource::s_Instance->camera;
        // load quad
        {
            const std::string hierarchyXml =R"(
            function button(width,height,tag,color,id)
                return {
                    id=id,
                    type="quad",
                    width=width,
                    height=height,
                    color=color,
                    content={
                        {
                            type="text",
                            width="100%",
                            height="100%",
                            size=24,
                            text=tag,
                        }
                    }
                }
            end
            return {
                type="grid",
                width="100%",
                height="100%",
                content={
                    button("10%","10%","btn1",{0.4,0.5,0.6,1.0},"btn1"),
                    {
                        type="text",
                        width="50%",
                        height="100%",
                        size=36,
                        text=[[I can swallow glass without any harm to myself
我能吞下玻璃而不伤身体
ガラスを飲み込んでも何ら害はない
Ich kann Glas schlucken, ohne mir selbst zu schaden]]
                    },
                },
            }
            )";
            UI::Lua::HierarchyLoader loader;
            loader.PushNodeCreator<UI::Lua::QuadNodeCreator>("quad");
            loader.PushNodeCreator<UI::Lua::GridNodeCreator>("grid");
            loader.PushNodeCreator<UI::Lua::TextNodeCreator>("text");
            auto err = loader.LoadHierarchy(m_Hierarchy, hierarchyXml);
            if (err)
            {
                std::println("Error loading hierarchy: {}", err.value());
                return;
            }
        }
        // add btn1 mouse callback
        do{
            auto* node=m_Hierarchy.GetNodeById("btn1");
            if (!node)
            {
                Debug::Log::Error("Node btn1 not found");
                break;
            }
            auto& mouseComponent=m_Hierarchy.AddComponent<UI::MouseComponent>(node);
            mouseComponent.onClick = [node]() {
                Debug::Log::Debug("Clicked on btn1 node");
            };

        }while(false);
    }

    virtual void OnEvent(Event& event) override
    {
        m_Hierarchy.OnEvent(event);
    }

private:
    UI::Hierarchy m_Hierarchy;
    Vec2f m_ScreenSize;
};
