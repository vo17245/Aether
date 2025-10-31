#pragma once
#include "AetherEditor/UIComponent/Notify.h"
#include "AddMaterialDataPanel.h"
#include "AetherEditor/FileWatcher.h"
#include <UI/FileDialog.h>
#include <AetherEditor/Utils/FileIsInDirectory.h>
using namespace Aether;
namespace AetherEditor::UI
{

class MaterialPanel
{
public:
    void Draw()
    {
        if (!m_Open)
        {
            return;
        }
        if (m_BringToFocused)
        {
            ImGui::SetNextWindowFocus();
            m_BringToFocused = false;
        }
        ImGui::Begin("Material Panel", nullptr);
        // dir
        {
            ImGui::PushID(0);
            if (ImGui::Button("Change"))
            {
                auto res = Aether::UI::SyncSelectDirectroy();
                if (res.has_value())
                {
                    m_Material.dir = res.value();
                    Notify::Info("Change material directory to " + m_Material.dir, 3.0f);
                    WatchShader();
                }
                else
                {
                    Notify::Warning("Failed to select directory: " + res.error(), 3.0f);
                }
            }
            ImGui::SameLine();
            ImGui::PopID();
            ImGui::Text("Directory: %s", m_Material.dir.c_str());
        }

        // shader
        {
            ImGui::PushID(1);
            if (ImGui::Button("Change"))
            {
                auto res = Aether::UI::SyncSelectFileEx({.defaultPath = m_Material.dir  });
                if (res.has_value())
                {
                    
                    m_Material.vertPath = Aether::Filesystem::Path(res.value()).Filename();
                    Notify::Info("Change vertext shader to " + m_Material.vertPath, 3.0f);
                }
                else
                {
                    Notify::Warning("Failed to select file: " + res.error(), 3.0f);
                }
            }
            ImGui::SameLine();
            ImGui::PopID();
            ImGui::Text("Vertex Shader: %s", m_Material.vertPath.c_str());
        }
        {
            ImGui::PushID(2);
            if (ImGui::Button("Change"))
            {
                auto res = Aether::UI::SyncSelectFileEx({.defaultPath = m_Material.dir  });
                if (res.has_value())
                {
                    m_Material.fragPath = Aether::Filesystem::Path(res.value()).Filename();
                    Notify::Info("Change fragment shader to " + m_Material.fragPath, 3.0f);
                }
                else
                {
                    Notify::Warning("Failed to select file: " + res.error(), 3.0f);
                }
            }
            ImGui::SameLine();
            ImGui::PopID();
            ImGui::Text("Fragment Shader: %s", m_Material.fragPath.c_str());
        }
        // material data
        for (size_t i = 0; i < m_DataPanels.size(); ++i)
        {
            m_DataPanels[i].Draw((int)i);
        }
        if (ImGui::Button("Add"))
        {
            if (addPanel.IsOpened())
            {
                addPanel.BringToFocused();
            }
            else
            {
                addPanel.Open();
            }
        }
        addPanel.Draw();
        if(ImGui::Button("Test"))
        {
            TestReflection();
        }
        ImGui::End();
    }
    void TestReflection();
    MaterialPanel()
    {
        addPanel.SetOnAdd([this](MaterialData& data) { OnMaterialDataAdd(data); });
    }
    void BringToFocused()
    {
        m_BringToFocused = true;
    }
    bool IsOpened()
    {
        return m_Open;
    }
    void Open()
    {
        m_Open = true;
    }
    void Close()
    {
        m_Open = false;
    }
    void OnUpdate(float deltaSec)
    {
        for (auto panel : m_DataToDelete)
        {
            auto iter = std::find_if(m_DataPanels.begin(), m_DataPanels.end(),
                                     [&panel](const MaterialDataPanel& p) { return &p == panel; });
            if (iter != m_DataPanels.end())
            {
                for (auto iter1 = m_Material.datas.begin(); iter1 != m_Material.datas.end(); ++iter1)
                {
                    if (iter1->get() == panel->GetData())
                    {
                        m_Material.datas.erase(iter1);
                        break;
                    }
                }
                m_DataPanels.erase(iter);
            }
            else
            {
                assert(false && "MaterialDataPanel not found");
            }
        }
        m_DataToDelete.clear();
    }
    void WatchShader()
    {
        if (m_WatchID.has_value())
        {
            FileWatcher::RemoveWatch(m_WatchID.value());
        }
        m_WatchID=FileWatcher::WatchDirectory(
            m_Material.dir, true,
            [this](efsw::WatchID watchId, const std::string& dir, const std::string& filename, efsw::Action action,
                   std::string oldFilename) { OnFileChanged(watchId, dir, filename, action, oldFilename); });
    }
    void DisableWatchShader()
    {
        if (m_WatchID.has_value())
        {
            FileWatcher::RemoveWatch(m_WatchID.value());
        }
    }

private:
    void OnFileChanged(efsw::WatchID watchId, const std::string& dir, const std::string& filename, efsw::Action action,
                       std::string oldFilename)
    {
        if (watchId == m_WatchID.value() && action == efsw::Action::Modified)
        {
            if (filename == m_Material.vertPath || filename == m_Material.fragPath)
            {
                OnShaderChanged();
            }
        }
    }
    void OnShaderChanged()
    {
        Notify::Debug("Shader file changed: " + m_Material.vertPath + " or " + m_Material.fragPath, 3.0f);
    }

private:
    void OnMaterialDataDelete(MaterialDataPanel& panel)
    {
        m_DataToDelete.push_back(&panel);
    }
    void OnMaterialDataAdd(MaterialData& data)
    {
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
        m_DataPanels.back().SetOnDelete([this](MaterialDataPanel* panel) { OnMaterialDataDelete(*panel); });
    }

private:
    AddMaterialDataPanel addPanel;
    Material m_Material;
    std::vector<MaterialDataPanel> m_DataPanels;
    bool m_BringToFocused = false;
    bool m_Open = false;
    std::vector<MaterialDataPanel*> m_DataToDelete;

private:
    std::optional<efsw::WatchID> m_WatchID;
};
}