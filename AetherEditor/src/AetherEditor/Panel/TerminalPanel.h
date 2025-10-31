#pragma once
#include <ImGui/Core/imgui.h>
#include <string>
namespace AetherEditor::UI
{

class TerminalPanel
{
public:
    void Draw()
    {
        ImGui::Begin(m_Title.c_str());
        ImGui::End();
    }
private:
    std::string m_Title = "Terminal";
};
}