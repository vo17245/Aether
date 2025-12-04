#pragma once
#include <string>
#include <ImGui/NodeEditor/imgui_node_editor.h>
#include <vector>
#include <AetherEditor/UIComponent/NodeEditor/NodeEditor.h>
#include <AetherEditor/UIComponent/NodeEditor/Nodes/MessageNode.h>
#include <AetherEditor/UIComponent/NodeEditor/Nodes/PrintStringNode.h>

using namespace Aether;
namespace AetherEditor::UI
{

class NodeEditorPanel
{
public:
    
    void Draw()
    {
        ImGui::Begin("NodeEditor Panel");
        m_NodeEditor.Draw();
        ImGui::End();
    }
    void Init(const std::string& configPath)
    {
        m_NodeEditor.Init();
        m_NodeEditor.RegisterNodeType<ImGuiComponent::NodeEditorTest::MessageNode>("Message");
        m_NodeEditor.RegisterNodeType<ImGuiComponent::NodeEditorTest::PrintStringNode>("PrintString");
    }

private:
    ImGuiComponent::NodeEditor m_NodeEditor;
};
}