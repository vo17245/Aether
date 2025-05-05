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
        renderer.GetCamera().target = Vec2f(m_ScreenSize.x() / 2, m_ScreenSize.y() / 2);
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
        m_ScreenSize.x() = window->GetSize().x();
        m_ScreenSize.y() = window->GetSize().y();

        UI::QuadSystem* quadSystem = new UI::QuadSystem();
        quadSystem->renderer = ApplicationResource::s_Instance->renderer.get();
        m_Hierarchy.AddSystem(quadSystem);
        // InitHierarchy();
        InitHierarchyFromXml();
    }
    void InitHierarchyFromXml()
    {
        const std::string hierarchyXml = std::format(R"(
        <quad width="{}" height="{}" color="0,0,0,1">
            <quad width="110" height="110" color="1,0,1,0">
                <quad width="100" height="100" color="1,0,1,1"/>
            </quad>
            <quad width="110" height="110" color="1,0,1,0">
                <quad width="100" height="100" color="1,0,1,1"/>
            </quad>
        </quad>

        )",
                                                     m_ScreenSize.x(), m_ScreenSize.y());
        UI::HierarchyLoader loader;
        loader.PushNodeCreator<UI::QuadNodeCreator>("quad");
        auto err = loader.LoadHierarchy(m_Hierarchy, hierarchyXml);
        if (err)
        {
            std::println("Error loading hierarchy: {}", err.value());
            return;
        }
        auto& renderer = *ApplicationResource::s_Instance->renderer;
        m_Hierarchy.RebuildLayout(m_ScreenSize, renderer.GetCamera().far);
    }
    void InitHierarchy()
    {
        {
            m_Root = m_Hierarchy.CreateNode();
            auto& base = m_Hierarchy.GetScene().GetComponent<UI::BaseComponent>(m_Root->id);
            base.position = Vec2f(0, 0);
            base.size = m_ScreenSize;
            m_Hierarchy.SetRoot(m_Root);
        }
        auto createQuad = [&](const Vec2f& size, const Vec4f& color) -> UI::Node* {
            auto* node = m_Hierarchy.CreateNode();
            auto& base = m_Hierarchy.GetScene().GetComponent<UI::BaseComponent>(node->id);
            UI::QuadDesc desc;
            desc.color = color;
            m_Hierarchy.GetScene().AddComponent<UI::QuadComponent>(node->id, desc);
            base.size = size;
            return node;
        };
        auto createGrid = [&](const Vec2f& size) -> UI::Node* {
            auto* node = m_Hierarchy.CreateNode();
            auto& base = m_Hierarchy.GetScene().GetComponent<UI::BaseComponent>(node->id);
            base.size = size;
            return node;
        };
        {
            auto* grid = createGrid({m_ScreenSize.x(), m_ScreenSize.y() * 0.1});
            auto* node = createQuad({m_ScreenSize.x(), m_ScreenSize.y() * 0.1 - 10}, Vec4f(1, 0, 0, 1));
            grid->children.push_back(node);
            m_Root->children.push_back(grid);
        }
        {
            auto* grid = createGrid({m_ScreenSize.x(), m_ScreenSize.y() * 0.8});
            auto* node = createQuad({100, 100}, Vec4f(1, 0, 0, 1));
            grid->children.push_back(node);
            m_Root->children.push_back(grid);
        }
        {
            auto* grid = createGrid({m_ScreenSize.x(), m_ScreenSize.y() * 0.1});
            auto* grid1 = createGrid({m_ScreenSize.x(), 10});
            grid->children.push_back(grid1);
            auto* node = createQuad({m_ScreenSize.x(), m_ScreenSize.y() * 0.1 - 10},
                                    Vec4f(0, 1, 0, 1));
            grid->children.push_back(node);
            m_Root->children.push_back(grid);
        }
        auto& renderer = *ApplicationResource::s_Instance->renderer;
        m_Hierarchy.RebuildLayout(m_ScreenSize, renderer.GetCamera().far);
    }

    virtual void OnEvent(Event& event) override
    {
    }

private:
    UI::Hierarchy m_Hierarchy;
    Vec2f m_ScreenSize;
    UI::Node* m_Root = nullptr;
};
