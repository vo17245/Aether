#pragma once
#include "Window/Layer.h"
#include "Window/Window.h"
#include "Window/WindowContext.h"
#include "Render/Mesh/VkMesh.h"
#include "Render/Mesh/Geometry.h"
#include "Render/Shader/Compiler.h"
#include "Render/Utils.h"
#include <print>
#include "Entry/Application.h"
#include <Render/RenderGraph/RenderGraph.h>
#include <IO/Image.h>
#include <Debug/Log.h>
#include "Material.h"
using namespace Aether;
#include "Notify.h"
#include "Panel/MaterialPanel.h"
#include "FileWatcher.h"
#include "Panel/ScenePanel.h"
#include <UI/Render/LoadTextureToLinear.h>
#include "Utils/LoadTexture.h"
#include "Panel/TerminalPanel.h"
#include "Panel/MaterialEditorPanel.h"
#include "Panel/MainWindow.h"
#include "Panel/NodeEditorPanel.h"
class ImGuiLayer : public Layer
{
public:
    ~ImGuiLayer()
    {
    }

    virtual void OnAttach(Window* window) override
    {
        m_Window = window;
        m_MaterialEditorPanel.Init();
        m_MainWindow.SetOsWindow(window);
        m_MainWindow.SetNodeEditorViewToggle([this]() { m_ShowNodeEditorPanel = !m_ShowNodeEditorPanel; });
        m_MainWindow.SetMaterialEditorViewToggle([this]() { m_ShowMaterialEditorPanel = !m_ShowMaterialEditorPanel; });
        m_NodeEditorPanel.Init("tmp/node_editor_panel.json");
    }
    virtual void OnUpdate(float sec) override
    {
        Notify::Update(sec);
        m_MaterialEditorPanel.OnUpdate(sec);
    }
    virtual bool NeedRebuildRenderGraph() override
    {
        return m_NeedRebuild;
    }

    virtual void OnImGuiUpdate() override
    {
        m_MainWindow.DrawBegin();
        if (m_ShowNodeEditorPanel)
        {
            m_NodeEditorPanel.Draw();
        }
        if (m_ShowMaterialEditorPanel)
        {
            m_MaterialEditorPanel.Draw();
        }
        Notify::Draw();
        m_MainWindow.DrawEnd();
    }

private:
    bool m_NeedRebuild;
    Window* m_Window = nullptr;

    MainWindow m_MainWindow;
    MaterialEditorPanel m_MaterialEditorPanel;
    NodeEditorPanel m_NodeEditorPanel;
    bool m_ShowNodeEditorPanel = false;
    bool m_ShowMaterialEditorPanel = false;
};