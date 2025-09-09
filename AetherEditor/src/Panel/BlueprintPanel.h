#pragma once
#include <string>
#include <ImGui/NodeEditor/imgui_node_editor.h>
#include <vector>
#include <UIComponent/Blueprint/Blueprint.h>
using namespace Aether;
class BlueprintPanel
{
public:
    
    void Draw()
    {
        ImGui::Begin("Blueprint Panel");
        m_Blueprint.Draw();
        ImGui::End();
    }
    void Init(const std::string& configPath)
    {
        m_Blueprint.Init();
    }

private:
    ImGuiComponent::Blueprint m_Blueprint;
};