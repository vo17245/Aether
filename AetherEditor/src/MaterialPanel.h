#pragma once
#include "Notify.h"
#include "AddMaterialDataPanel.h"
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
        ImGui::Begin("Material Panel");
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
        ImGui::End();
    }
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
};