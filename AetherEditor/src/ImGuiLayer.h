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
class ImGuiLayer : public Layer
{
public:
    ~ImGuiLayer()
    {
    }

    virtual void OnAttach(Window* window) override
    {
        m_Window = window;
        // Notify::Error("This is an error message", 10.0f);
        // Notify::Info("This is an info message", 10.0f);
        // Notify::Warning("This is an warning message", 10.0f);
        m_MaterialEditorPanel.Init();
        m_MainWindow.SetOsWindow(window);
    }
    virtual void OnUpdate(float sec) override
    {
        Notify::Update(sec);
        m_MaterialEditorPanel.OnUpdate(sec);
    }
    virtual void RegisterRenderPasses(RenderGraph::RenderGraph& renderGraph) override
    {
    }
    virtual bool NeedRebuildRenderGraph() override
    {
        return m_NeedRebuild;
    }

    virtual void OnImGuiUpdate() override
    {
        m_MainWindow.DrawBegin();
        m_MaterialEditorPanel.Draw();
        Notify::Draw();
        m_MainWindow.DrawEnd();
    }
    

private:
    bool m_NeedRebuild;
    Window* m_Window = nullptr;

    MainWindow m_MainWindow;
    MaterialEditorPanel m_MaterialEditorPanel;
};