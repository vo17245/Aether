#pragma once
#include "Panel/MaterialPanel.h"
#include "Panel/ScenePanel.h"
#include "Panel/TerminalPanel.h"
#include "Utils/LoadTexture.h"
class MaterialEditorPanel
{
public:
    void Init()
    {
        m_MaterialPanel.Open();
        m_DummyScene = Utils::LoadSrgbTexture("Assets/bundle/Images/logo.png").value();
        m_ScenePanel.SetTexture(m_DummyScene);
    }
    void OnUpdate(float sec)
    {
        m_MaterialPanel.OnUpdate(sec);
    }
    void Draw()
    {
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));

        ImGui::Begin(m_Title.c_str(), nullptr);
        ImGui::PopStyleVar();

        ImGuiID dockspace_id = ImGui::GetID("Material Editor");
        ImGui::DockSpace(dockspace_id, ImVec2(0, 0));
        m_MaterialPanel.Draw();
        m_ScenePanel.Draw();
        m_TerminalPanel.Draw();
        ImGui::End();
    }

private:
    std::string m_Title = "Material Editor";
    ScenePanel m_ScenePanel;
    DeviceTexture m_DummyScene;
    TerminalPanel m_TerminalPanel;
    MaterialPanel m_MaterialPanel;
};