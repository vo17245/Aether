#pragma once
#include "Panel.h"

class AssetTypeSearchPanel : public Panel
{
public:
    AssetTypeSearchPanel()
    {
        m_AssetTypes = {"Texture",   "Mesh",   "Material", "Shader", "Audio",
                        "Animation", "Prefab", "Script",   "Font",   "Scene"};
        m_FilteredAssetTypes = m_AssetTypes;
    }
    std::string& GetTitle() override
    {
        return m_Title;
    }
    void SetVisible(bool visible) override
    {
        m_Visible = visible;
    }
    bool GetVisible() override
    {
        return m_Visible;
    }
    void OnImGuiUpdate() override
    {
        if (!m_Visible)
            return;
        if (m_SetPositionOnce)
        {
            ImGui::SetNextWindowPos(ImVec2(m_Position.x(), m_Position.y()), ImGuiCond_Always);
            m_SetPositionOnce = false;
        }
        if (m_SetSizeOnce)
        {
            ImGui::SetNextWindowSize(ImVec2(m_Size.x(), m_Size.y()), ImGuiCond_Always);
            m_SetSizeOnce = false;
        }
        ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize
                                         | ImGuiWindowFlags_NoCollapse|ImGuiWindowFlags_NoDocking;
        
        // set no title no resize no move no collapse
        ImGui::Begin(m_Title.c_str(), nullptr, window_flags);
        if (ImGui::InputText("##", m_SearchBuffer, sizeof(m_SearchBuffer)))
        {
            FilterAssetTypes();
        }
        ImGui::Separator();
        for (const auto& type : m_FilteredAssetTypes)
        {
            if (ImGui::Selectable(type.c_str())) 
            {
                OnAssetTypeSelected(type);
                m_Visible=false;
            }
        }

        if (!ImGui::IsWindowFocused())
        {
            m_Visible = false;
        }
        ImGui::End();
    }
    void SetPosition(float x, float y) override
    {
        m_Position = Vec2f(x, y);
        m_SetPositionOnce = true;
    }
    void SetSize(float width, float height) override
    {
        m_Size = Vec2f(width, height);
        m_SetSizeOnce = true;
    }
private:
    void OnAssetTypeSelected(const std::string& type)
    {
    }
private:
    std::string m_Title = "Asset Type Search";
    bool m_Visible = false;
    Vec2f m_Position;
    Vec2f m_Size;
    char m_SearchBuffer[256]{0};
    std::vector<std::string> m_AssetTypes;
    std::vector<std::string> m_FilteredAssetTypes;
    void FilterAssetTypes()
    {
        m_FilteredAssetTypes.clear();
        std::string searchStr(m_SearchBuffer);
        for (const auto& type : m_AssetTypes)
        {
            if (type.find(searchStr) != std::string::npos)
            {
                m_FilteredAssetTypes.push_back(type);
            }
        }
    }
    bool m_SetPositionOnce = false;
    bool m_SetSizeOnce = false;
};