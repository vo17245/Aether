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
#include <UI/Hierarchy/Loader/HierarchyXmlLoader.h>
#include <UI/Hierarchy/Loader/NodeCreator.h>
#include <UI/Hierarchy/Loader/BuiltinXmlNodeCreator.h>
#include <UI/Hierarchy/System/Text.h>
#include <UI/Hierarchy/System/Mouse.h>
#include <UI/Hierarchy/System/InputText.h>
using namespace Aether;
class HierarchyLayer : public Layer
{
public:
    ~HierarchyLayer()
    {
    }
    virtual void OnDetach()override
    {
        m_Hierarchy.reset();
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
        m_Hierarchy->OnRender(commandBuffer,  framebuffer,
                             m_ScreenSize);
        commandBuffer.EndRenderPass();
    }
    virtual void OnAttach(Window* window) override
    {
        m_TextureCache=CreateScope<UI::TextureCache>(
            ApplicationResource::s_Instance->renderResource.m_StagingBuffer.get());
        Debug::Log::Debug("HierarchyLayer Attach");
        m_ScreenSize.x() = window->GetSize().x();
        m_ScreenSize.y() = window->GetSize().y();
        m_Scene=CreateScope<Scene>();
        m_Hierarchy=CreateScope<UI::Hierarchy>(m_Scene.get());
        // quad
        UI::QuadSystem* quadSystem = new UI::QuadSystem(*m_TextureCache);
        quadSystem->SetCamera(&m_Hierarchy->GetCamera());
        quadSystem->renderer = ApplicationResource::s_Instance->renderer.get();
        m_Hierarchy->AddSystem(quadSystem);
        // text
        UI::TextSystem* textSystem = UI::TextSystem::Create(ApplicationResource::s_Instance->renderPass,
                                                            *ApplicationResource::s_Instance->renderResource.m_DescriptorPool);
        textSystem->SetDescriptorPool(ApplicationResource::s_Instance->renderResource.m_DescriptorPool.get());
        textSystem->SetCamera(&m_Hierarchy->GetCamera());
        textSystem->AddAssetDir("Assets");
        m_Hierarchy->AddSystem(textSystem);
        // mouse
        UI::MouseSystem* mouseSystem = new UI::MouseSystem();
        m_Hierarchy->AddSystem(mouseSystem);
        // input text
        UI::InputTextSystem* inputTextSystem = new UI::InputTextSystem();
        m_Hierarchy->AddSystem(inputTextSystem);
        // load xml
        InitHierarchyFromXml();
    }
    void InitHierarchyFromXml()
    {
        auto& camera = ApplicationResource::s_Instance->camera;
        // load quad
        {
            const std::string hierarchyXml = std::format(R"(
        <quad width="{}" height="{}" color="0,0,0,1">
            <quad width="110" height="110" color="1,0,1,0">
                <quad width="100" height="100" color="1,0,1,1">
                    <text width="100" height="100" world_size="36">quad1</text>
                </quad>
            </quad>
            <quad width="110" height="110" color="1,0,1,0">
                <quad width="100" height="100" color="1,0,1,1">
                    <text width="100" height="100" world_size="36">quad2</text>
                </quad>
            </quad>
            <text width="400" height="400" world_size="33">
<![CDATA[我能吞下玻璃而不伤身体
I can swallow glass without any harm to myself
ガラスを飲み込んでも何ら害はない
Ich kann Glas schlucken, ohne mir selbst zu schaden
]]>
            </text>
        </quad>

        )",
                                                         m_ScreenSize.x(), m_ScreenSize.y());
            UI::Xml::HierarchyLoader loader;
            loader.PushNodeCreator<UI::Xml::QuadNodeCreator>("quad");
            loader.PushNodeCreator<UI::Xml::TextNodeCreator>("text");
            auto err = loader.LoadHierarchy(*m_Hierarchy, hierarchyXml);
            if (err)
            {
                std::println("Error loading hierarchy: {}", err.value());
                return;
            }
        }
        // add mouse entity (debug)
        {
            auto* node = m_Hierarchy->CreateNode();
            // set position and size
            auto& bc = m_Hierarchy->GetComponent<UI::BaseComponent>(node);
            bc.position = Vec2f(0, 400);
            bc.size = Vec2f(100, 100);
            // add quad component
            UI::QuadDesc desc;
            desc.color = Vec4f(1, 0, 0, 1); // red color
            auto& quadComponent = m_Hierarchy->AddComponent<UI::QuadComponent>(node, desc);
            // add mouse component
            UI::MouseComponent mouseComponent;
            mouseComponent.onClick = [&quadComponent]() {
                Debug::Log::Debug("Clicked on mouse node");
            };
            mouseComponent.onPress = [&quadComponent]() {
                quadComponent.quad.SetColor(Vec4f(0.5, 0, 0, 1));
            };
            mouseComponent.onRelease = [&quadComponent]() {
                quadComponent.quad.SetColor(Vec4f(1, 0, 0, 1));
            };
            m_Hierarchy->AddComponent<UI::MouseComponent>(node, std::move(mouseComponent));
            // add text component
            auto& textComponent = m_Hierarchy->AddComponent<UI::TextComponent>(node, "Mouse Node");
            // add input text component
            m_Hierarchy->AddComponent<UI::InputTextComponent>(node);
        }

        m_Hierarchy->RebuildLayout();
    }

    virtual void OnEvent(Event& event) override
    {
        m_Hierarchy->OnEvent(event);
    }

private:
    std::unique_ptr<UI::Hierarchy> m_Hierarchy;
    std::unique_ptr<Scene> m_Scene;
    Vec2f m_ScreenSize;
    UI::Node* m_Root = nullptr;
    std::unique_ptr<UI::TextureCache> m_TextureCache;
};
