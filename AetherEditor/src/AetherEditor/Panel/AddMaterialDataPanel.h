#pragma once
#include "AetherEditor/Notify.h"
#include "AetherEditor/Material.h"
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
        ImGui::SameLine();
        
        ImGui::PushID(id);
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.1f, 0.1f, 1.0f));
        if(ImGui::Button("Delete"))
        {
            if(m_OnDelete)
            {
                m_OnDelete(this);
            }
        }
        ImGui::PopStyleColor();
        DrawMaterialDataValue(*m_Data);
        ImGui::PopID();
    }
    void SetOnDelete(std::function<void(MaterialDataPanel*)>&& onDelete)
    {
        m_OnDelete = std::move(onDelete);
    }
    MaterialData* GetData()
    {
        return m_Data;
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
    std::function<void(MaterialDataPanel*)> m_OnDelete;
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
        if(m_BringToFocused)
        {
            ImGui::SetNextWindowFocus();
            m_BringToFocused=false;
        }
        // get parent window size
        ImGui::Begin("Add Material Data", &m_Open, ImGuiWindowFlags_NoDocking);
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
        switch(type)
        {
            case MaterialDataType::Float:
            {
                auto& d = static_cast<MaterialFloat&>(*m_Data);
                ImGui::InputFloat("Value", &d.value);
                ImGui::InputFloat("Min", &d.min);
                ImGui::InputFloat("Max", &d.max);
            }
            break;
            case MaterialDataType::Vec2:
            {
                auto& d = static_cast<MaterialVec2&>(*m_Data);
                ImGui::InputFloat2("Value", d.value.data());
                ImGui::InputFloat2("Min", d.min.data());
                ImGui::InputFloat2("Max", d.max.data());
            }
            break;
            case MaterialDataType::Vec3:
            {
                auto& d = static_cast<MaterialVec3&>(*m_Data);
                ImGui::InputFloat3("Value", d.value.data());
                ImGui::InputFloat3("Min", d.min.data());
                ImGui::InputFloat3("Max", d.max.data());
            }
            break;
            case MaterialDataType::Vec4:
            {
                auto& d = static_cast<MaterialVec4&>(*m_Data);
                ImGui::InputFloat4("Value", d.value.data());
                ImGui::InputFloat4("Min", d.min.data());
                ImGui::InputFloat4("Max", d.max.data());
            }
            break;
            default:
                assert(false && "unknown type");
                break;
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
    bool IsOpened()
    {
        return m_Open;
    }
    void BringToFocused()
    {
        m_BringToFocused = true;
    }

private:
    Scope<MaterialData> m_Data;
    MaterialDataType m_LastType;
    int m_TypeIndex = 0;
    char m_Name[128] = "NewData";
    std::function<void(MaterialData&)> m_OnAdd;
    bool m_Open = false;
    bool m_BringToFocused = false;
};