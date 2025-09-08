#pragma once
#include <string>
#include <ImGui/NodeEditor/imgui_node_editor.h>
#include <vector>
class BlueprintPanel
{
public:
    struct LinkInfo
    {
        ImGui::NodeEditor::LinkId Id;
        ImGui::NodeEditor::PinId  InputId;
        ImGui::NodeEditor::PinId  OutputId;
    };
    void Draw();
    void Init(const std::string& configPath);
    void Destroy()
    {
        ImGui::NodeEditor::DestroyEditor(m_Context);
    }
    ~BlueprintPanel()
    {
        if (m_Context)
        {
            Destroy();
        }
    }
    
private:
    void ImGuiEx_BeginColumn()
    {
        ImGui::BeginGroup();
    }

    void ImGuiEx_NextColumn()
    {
        ImGui::EndGroup();
        ImGui::SameLine();
        ImGui::BeginGroup();
    }
    void ImGuiEx_EndColumn()
    {
        ImGui::EndGroup();
    }
private:
    ImGui::NodeEditor::EditorContext* m_Context = nullptr;
    std::vector<LinkInfo> m_Links;
    bool    m_FirstFrame = true;    
    int     m_NextLinkId = 100;
};