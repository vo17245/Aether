#pragma once
#include <string>
#include <ImGui/NodeEditor/imgui_node_editor.h>
#include <vector>
#include <UIComponent/Blueprint/Blueprint.h>
#include <UIComponent/Blueprint/Nodes/MessageNode.h>
#include <UIComponent/Blueprint/Nodes/PrintStringNode.h>

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
        m_Blueprint.RegisterNodeType<ImGuiComponent::BlueprintTest::MessageNode>("Message");
        m_Blueprint.RegisterNodeType<ImGuiComponent::BlueprintTest::PrintStringNode>("PrintString");
    }

private:
    ImGuiComponent::Blueprint m_Blueprint;
};