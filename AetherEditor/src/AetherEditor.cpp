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
#include "MaterialPanel.h"
#include "FileWatcher.h"



class ImGuiLayer : public Layer
{
public:
    ~ImGuiLayer()
    {
    }

    virtual void OnAttach(Window* window) override
    {
        m_Window = window;
        Notify::Error("This is an error message", 10.0f);
        Notify::Info("This is an info message", 10.0f);
        Notify::Warning("This is an warning message", 10.0f);
        m_MaterialPanel.Open();
    }
    virtual void OnUpdate(float sec) override
    {
        Notify::GetSingleton().Update(sec);
        m_MaterialPanel.OnUpdate(sec);
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
        Vec2i size = m_Window->GetSize();
        // full screen
        ImGui::SetNextWindowSize(ImVec2((float)size.x(), (float)size.y()));
        // no title bar
        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoNav
                                        | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoTitleBar|ImGuiWindowFlags_NoResize;

        ImGui::Begin("Docking", nullptr, window_flags);
        m_MaterialPanel.Draw();
        Notify::Draw();
        ImGui::End();
    }

private:
    bool m_NeedRebuild;
    Window* m_Window = nullptr;
    MaterialPanel m_MaterialPanel;
    float m_Float;
};

class AetherEditor : public Application
{
public:
    virtual void OnInit(Window& window) override
    {
        ImGuiApi::EnableDocking();
        auto* imguiLayer = new ImGuiLayer();
        m_Layers.push_back(imguiLayer);
        window.PushLayer(imguiLayer);
        FileWatcher::Start();
    }
    virtual void OnShutdown() override
    {
        for (auto* layer : m_Layers)
        {
            delete layer;
        }
    }
    virtual void OnFrameBegin() override
    {
    }
    virtual const char* GetName() const override
    {
        return "ImGuiDemo";
    }

private:
    std::vector<Layer*> m_Layers;
    Window* m_MainWindow;
};
DEFINE_APPLICATION(AetherEditor);