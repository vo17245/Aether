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
enum class MessageType
{
    Info,
    Warning,
    Error
};
struct Message
{
    MessageType type;
    std::string content;
    float ttl; // time to live in seconds
};
class Notify
{
public:
    void Update(float sec)
    {
        for (auto it = m_Messages.begin(); it != m_Messages.end();)
        {
            it->ttl -= sec;
            if (it->ttl <= 0.0f)
            {
                it = m_Messages.erase(it);
            }
            else
            {
                ++it;
            }
        }
    }

    static Notify& GetSingleton()
    {
        static Notify instance;
        return instance;
    }
    static void Error(const std::string& content, float ttl = 5.0f)
    {
        GetSingleton().ErrorImpl(content, ttl);
    }
    static void Warning(const std::string& content, float ttl = 5.0f)
    {
        GetSingleton().WarningImpl(content, ttl);
    }
    static void Info(const std::string& content, float ttl = 5.0f)
    {
        GetSingleton().InfoImpl(content, ttl);
    }
    static void Draw()
    {
        GetSingleton().DrawImpl();
    }

private:
    void DrawImpl()
    {
        if (m_Messages.empty())
        {
            return;
        }
        ImGui::SetNextWindowBgAlpha(0.3f); // Transparent background
        ImGui::Begin("Notifications", nullptr,
                     ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize
                         | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoMove);
        for (const auto& msg : m_Messages)
        {
            ImVec4 color;
            switch (msg.type)
            {
            case MessageType::Info:
                color = ImVec4(0.0f, 1.0f, 0.0f, 1.0f);
                break;
            case MessageType::Warning:
                color = ImVec4(1.0f, 1.0f, 0.0f, 1.0f);
                break;
            case MessageType::Error:
                color = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
                break;
            default:
                color = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
                break;
            }
            ImGui::PushStyleColor(ImGuiCol_Text, color);
            ImGui::TextUnformatted(msg.content.c_str());
            ImGui::PopStyleColor();
        }
        ImGui::End();
    }
    void ErrorImpl(const std::string& content, float ttl = 5.0f)
    {
        m_Messages.push_back({MessageType::Error, content, ttl});
    }
    void WarningImpl(const std::string& content, float ttl = 5.0f)
    {
        m_Messages.push_back({MessageType::Warning, content, ttl});
    }
    void InfoImpl(const std::string& content, float ttl = 5.0f)
    {
        m_Messages.push_back({MessageType::Info, content, ttl});
    }
    std::vector<Message> m_Messages;
};
class MaterialDataSetLimitPanel
{
public:
    MaterialDataSetLimitPanel(MaterialData* data) : m_Data(data)
    {
    }
    void Draw(int id)
    {
        if (!m_Open)
        {
            return;
        }
        ImGui::PushID(id);

        ImGui::PopID();
    }

private:
    MaterialData* m_Data = nullptr;
    bool m_Open = false;
};
class MaterialDataPanel
{
public:
    MaterialDataPanel(MaterialData* data) : m_Data(data)
    {
    }
    void Draw(int id)
    {
        ImGui::Text("%s", m_Data->name.c_str());
        ImGui::PushID(id);
        DrawMaterialDataValue(*m_Data);
        ImGui::PopID();
    }

private:
    void DrawMaterialDataValue(MaterialData& data)
    {
        switch (data.type)
        {
        case MaterialDataType::Float: {
            // slider
            auto& d = static_cast<MaterialFloat&>(data);

            ImGui::SliderFloat("x", &d.value, d.min, d.max);
        }
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
        default:
            assert(false && "unknown type");
            break;
        }
    }

private:
    MaterialData* m_Data = nullptr;
};
class AddMaterialDataPanel
{
public:
    AddMaterialDataPanel()
    {
        m_Data = CreateScope<MaterialFloat>();
        m_LastType = MaterialDataType::Float;
    }
    void Draw()
    {
        if (!m_Open)
        {
            return;
        }
        // get parent window size
        ImGui::Begin("Add Material Data", &m_Open);
        if (ImGui::InputText("Name", m_Name, sizeof(m_Name)))
        {
            m_Data->name = m_Name;
        }
        ImGui::Combo("Type", (int*)&m_TypeIndex, "Float\0Vec2\0Vec3\0Vec4\0");
        MaterialDataType type = TypeIndexToEnum(m_TypeIndex);
        if (type != m_LastType)
        {
            m_LastType = type;
            switch (type)
            {
            case MaterialDataType::Float:
                m_Data = CreateScope<MaterialFloat>();
                break;
            case MaterialDataType::Vec2:
                m_Data = CreateScope<MaterialVec2>();
                break;
            case MaterialDataType::Vec3:
                m_Data = CreateScope<MaterialVec3>();
                break;
            case MaterialDataType::Vec4:
                m_Data = CreateScope<MaterialVec4>();
                break;
            default:
                assert(false && "unknown type");
                m_Data = CreateScope<MaterialFloat>();
                break;
            }
        }
        if (ImGui::Button("Add"))
        {
            m_Open = false;
            if (m_OnAdd)
            {
                m_Data->name = m_Name;
                m_OnAdd(*m_Data);
            }
        }
        ImGui::End();
    }
    MaterialDataType TypeIndexToEnum(int index)
    {
        switch (index)
        {
        case 0:
            return MaterialDataType::Float;
        case 1:
            return MaterialDataType::Vec2;
        case 2:
            return MaterialDataType::Vec3;
        case 3:
            return MaterialDataType::Vec4;
        default:
            assert(false && "unknown type");
            return MaterialDataType::Float;
        }
    }
    void SetOnAdd(std::function<void(MaterialData&)>&& onAdd)
    {
        m_OnAdd = std::move(onAdd);
    }
    void Open()
    {
        m_Open = true;
    }

private:
    Scope<MaterialData> m_Data;
    MaterialDataType m_LastType;
    int m_TypeIndex = 0;
    char m_Name[128] = "NewData";
    std::function<void(MaterialData&)> m_OnAdd;
    bool m_Open = false;
};
class MaterialPanel
{
public:
    void Draw()
    {
        for (size_t i = 0; i < m_DataPanels.size(); ++i)
        {
            m_DataPanels[i].Draw((int)i);
        }
        if (ImGui::Button("Add"))
        {
            addPanel.Open();
        }
        addPanel.Draw();
    }
    MaterialPanel()
    {
        addPanel.SetOnAdd([this](MaterialData& data) {
            switch (data.type)
            {
            case MaterialDataType::Float:
                m_Material.datas.push_back(CreateScope<MaterialFloat>(static_cast<MaterialFloat&>(data)));
                break;
            case MaterialDataType::Vec2:
                m_Material.datas.push_back(CreateScope<MaterialVec2>(static_cast<MaterialVec2&>(data)));
                break;
            case MaterialDataType::Vec3:
                m_Material.datas.push_back(CreateScope<MaterialVec3>(static_cast<MaterialVec3&>(data)));
                break;
            case MaterialDataType::Vec4:
                m_Material.datas.push_back(CreateScope<MaterialVec4>(static_cast<MaterialVec4&>(data)));
                break;
            default:
                assert(false && "unknown type");
                break;
            }
            m_DataPanels.push_back(MaterialDataPanel(m_Material.datas.back().get()));
        });
    }

private:
    void OnMaterialDataAdd(MaterialData& data)
    {
    }

private:
    AddMaterialDataPanel addPanel;
    Material m_Material;
    std::vector<MaterialDataPanel> m_DataPanels;
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
        Notify::Error("This is an error message", 10.0f);
        Notify::Error("This is an error message", 10.0f);
        Notify::Error("This is an error message", 10.0f);
        Notify::Error("This is an error message", 10.0f);
    }
    virtual void OnUpdate(float sec) override
    {
        Notify::GetSingleton().Update(sec);
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
        m_MaterialPanel.Draw();
        ImGui::End();
        Notify::Draw();
    }

private:
    bool m_NeedRebuild;
    Window* m_Window = nullptr;
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