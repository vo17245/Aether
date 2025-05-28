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
#include <UI/Hierarchy/HierarchyLoader.h>
#include <UI/Hierarchy/NodeCreator.h>
#include <UI/Hierarchy/BuiltinNodeCreator.h>
#include <UI/Hierarchy/System/Text.h>
#include <UI/Hierarchy/System/Mouse.h>
#include <UI/Hierarchy/System/InputText.h>
#include <NodeCreators/Button.h>
#include "Query.h"
#include <UI/Hierarchy/Component/VisibilityRequest.h>
#include <UI/Hierarchy/System/VisibilityRequest.h>
using namespace Aether;
class MainPage : public Layer
{
public:
    ~MainPage()
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
        Debug::Log::Debug("MainPage Attach");
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
        // visibility request
        UI::VisibilityRequestSystem* visibilityRequestSystem = new UI::VisibilityRequestSystem();
        m_Hierarchy.AddSystem(visibilityRequestSystem);
        // load xml
        InitHierarchyFromXml();
    }
    void InitHierarchyFromXml()
    {
        auto& camera = ApplicationResource::s_Instance->camera;
        // load xml
        std::string path = std::string(Constants::AssetPath) + "/Pages/MainPage.xml";
        Filesystem::File file(path, Filesystem::Action::Read);
        assert(file.IsOpened() && "MainPage.xml not found");
        size_t fileSize = file.GetSize();
        std::string buffer(fileSize, 0);
        auto handle = file.GetHandle();
        Filesystem::Read(handle, std::span<uint8_t>((uint8_t*)buffer.data(), fileSize));
        {
            UI::HierarchyLoader loader;
            loader.PushNodeCreator<UI::QuadNodeCreator>("quad");
            loader.PushNodeCreator<UI::TextNodeCreator>("text");
            loader.PushNodeCreator<ButtonNodeCreator>("button");
            auto err = loader.LoadHierarchy(m_Hierarchy, buffer);
            if (err)
            {
                std::println("Error loading hierarchy: {}", err.value());
                return;
            }
        }
        // set callback
        {
            
            m_Doc.Parse(buffer.c_str());

            // file popup
            {
                auto* filePopup = query("/quad/quad[2]");
                // visibility request
                auto& vrc = m_Hierarchy.AddComponent<UI::VisibilityRequest>(filePopup);
                vrc.visible = false;
                vrc.processed = false;
                auto& bc = m_Hierarchy.GetComponent<UI::BaseComponent>(filePopup);
                bc.layoutEnabled=false;
            }
            // File button
            {
                auto* fileButton = query("/quad/quad/button[0]");
                auto& mc = m_Hierarchy.GetComponent<UI::MouseComponent>(fileButton);
                auto& quad = m_Hierarchy.GetComponent<UI::QuadComponent>(fileButton);
                // onPress

                mc.onPress = [&]() {
                    quad.quad.SetColor(Vec4f(0.1, 0.1, 0.1, 1));
                };
                // on release
                mc.onRelease = [&]() {
                    quad.quad.SetColor(Vec4f(0.5, 0.5, 0.5, 1));
                };
                auto* filePopup = query("/quad/quad[2]");
                auto& vrc = m_Hierarchy.GetComponent<UI::VisibilityRequest>(filePopup);
                auto& popupBase = m_Hierarchy.GetComponent<UI::BaseComponent>(filePopup);
                auto& base= m_Hierarchy.GetComponent<UI::BaseComponent>(fileButton);
                mc.onClick = [&]() {
                    Debug::Log::Debug("popup base enable layout: {}", popupBase.layoutEnabled);
                    // set pos
                    popupBase.position=Vec2f(500,500);
                    popupBase.z= base.z - 1;
                    // visibility request
                    
                    vrc.visible = !vrc.visible;
                    vrc.processed = false;
                    m_NeedRebuildLayout = true;
                };
            }
            // Edit button
            {
                auto* fileButton = query("/quad/quad/button[2]");
                auto& mc = m_Hierarchy.GetComponent<UI::MouseComponent>(fileButton);
                auto& quad = m_Hierarchy.GetComponent<UI::QuadComponent>(fileButton);
                // onPress

                mc.onPress = [&]() {
                    quad.quad.SetColor(Vec4f(0.1, 0.1, 0.1, 1));
                };
                // on release
                mc.onRelease = [&]() {
                    quad.quad.SetColor(Vec4f(0.5, 0.5, 0.5, 1));
                };
            }
            // View button
            {
                auto* fileButton = query("/quad/quad/button[1]");
                auto& mc = m_Hierarchy.GetComponent<UI::MouseComponent>(fileButton);
                auto& quad = m_Hierarchy.GetComponent<UI::QuadComponent>(fileButton);
                // onPress

                mc.onPress = [&]() {
                    quad.quad.SetColor(Vec4f(0.1, 0.1, 0.1, 1));
                };
                // on release
                mc.onRelease = [&]() {
                    quad.quad.SetColor(Vec4f(0.5, 0.5, 0.5, 1));
                };
            }
            
        }
        m_Hierarchy.RebuildLayout(m_ScreenSize, camera.far);
    }

    virtual void OnEvent(Event& event) override
    {
        m_Hierarchy.OnEvent(event);
    }
    virtual void OnUpdate(float sec) override
    {
        m_Hierarchy.OnUpdate(sec);
    }
    virtual void OnFrameBegin() override
    {
        if (m_NeedRebuildLayout)
        {
            m_Hierarchy.RebuildLayout(m_ScreenSize, ApplicationResource::s_Instance->camera.far);
            m_NeedRebuildLayout = false;
        }
    }
private:
    UI::Hierarchy m_Hierarchy;
    Vec2f m_ScreenSize;
    UI::Node* m_Root = nullptr;
    tinyxml2::XMLDocument m_Doc;
    std::function<UI::Node*(std::string_view)> query = [&](std::string_view path) {
        return QueryNode(m_Hierarchy, m_Doc, path);};
    bool m_NeedRebuildLayout = false; // 是否需要重新布局
};
