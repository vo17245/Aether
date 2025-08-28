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


static void DrawMaterialDataItem(MaterialData& data, int id)
{
    ImGui::Text("%s", data.name.c_str());
    ImGui::PushID(id);
    switch (data.type)
    {
    case MaterialDataType::Float: {
        // slider
        auto& d = static_cast<MaterialFloat&>(data);

        ImGui::SliderFloat("x", &d.value, d.min, d.max);
    }
    break;
    default:
        break;
    case MaterialDataType::Vec2: {
        auto& d = static_cast<MaterialVec2&>(data);
        ImGui::SliderFloat("x", d.value.data(), d.min.x(), d.max.x());
        ImGui::SliderFloat("y", d.value.data() + 1, d.min.y(), d.max.y());
    }
    break;
    case MaterialDataType::Vec3: {
        auto& d = static_cast<MaterialVec3&>(data);
        ImGui::SliderFloat("x", d.value.data(), d.min.x(), d.max.x());
        ImGui::SliderFloat("y", d.value.data() + 1, d.min.y(), d.max.y());
        ImGui::SliderFloat("z", d.value.data() + 2, d.min.z(), d.max.z());
    }
    break;
    case MaterialDataType::Vec4: {
        auto& d = static_cast<MaterialVec4&>(data);
        ImGui::SliderFloat("x", d.value.data(), d.min.x(), d.max.x());
        ImGui::SliderFloat("y", d.value.data() + 1, d.min.y(), d.max.y());
        ImGui::SliderFloat("z", d.value.data() + 2, d.min.z(), d.max.z());
        ImGui::SliderFloat("w", d.value.data() + 3, d.min.w(), d.max.w());
    }
    break;
    }
    ImGui::PopID();
}
struct MaterialPanel
{
    void Draw(Material& material)
    {
        for (size_t i = 0; i < material.datas.size(); ++i)
        {
            DrawMaterialDataItem(*material.datas[i], i);
        }
    }
};
class ImGuiLayer : public Layer
{
public:
    ~ImGuiLayer()
    {
    }

    virtual void OnAttach(Window* window) override
    {
        m_Window = window;
        m_Material.datas.push_back(CreateScope<MaterialFloat>());
        m_Material.datas.back()->name = "Roughness";
        static_cast<MaterialFloat*>(m_Material.datas.back().get())->value = 0.5f;
        m_Material.datas.push_back(CreateScope<MaterialVec3>());
        m_Material.datas.back()->name = "BaseColor";
        static_cast<MaterialVec3*>(m_Material.datas.back().get())->value = Vec3f(1.0f, 0.0f, 0.0f);
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
        ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove
                                        | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing
                                        | ImGuiWindowFlags_NoNav;

        ImGui::Begin("Test Window", nullptr, window_flags);
        ImGui::Text("Hello from ImGuiLayer");
        m_MaterialPanel.Draw(m_Material);
        ImGui::End();
    }

private:
    bool m_NeedRebuild;
    Window* m_Window = nullptr;
    Material m_Material;
    MaterialPanel m_MaterialPanel;
    float m_Float;
};

class ImGuiDemo : public Application
{
public:
    virtual void OnInit(Window& window) override
    {
        auto* testLayer = new ImGuiLayer();
        m_Layers.push_back(testLayer);
        window.PushLayer(testLayer);
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
DEFINE_APPLICATION(ImGuiDemo);