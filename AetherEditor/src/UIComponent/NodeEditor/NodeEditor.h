#pragma once
#include <ImGui/NodeEditor/imgui_node_editor.h>
#include <string>
#include <vector>
#include <functional>
#include <UIComponent/Image.h>
#include <variant>
#include "NodeEditorNode.h"
#include "NodeEditorIdAllocator.h"
namespace Aether::ImGuiComponent
{

class NodeEditor
{
public:
    using NodeCreator = std::function<NodeEditorNode*(NodeEditorIdAllocator& idAllocator)>;
public:
    ~NodeEditor()
    {
        Destroy();
    }

public:
    template<typename NodeType>
    void RegisterNodeType(const std::string& name)
    {
        m_NodeCreators[name] = [](NodeEditorIdAllocator& idAllocator) -> NodeType* {
            auto* node = new NodeType(idAllocator);
            return node;
        };
    }
    void Init();

    void Destroy();
    void Draw();

private:
    int GetNextId();

    ImGui::NodeEditor::LinkId GetNextLinkId();

    void TouchNode(ImGui::NodeEditor::NodeId id);

    float GetTouchProgress(ImGui::NodeEditor::NodeId id);

    void UpdateTouch();

    NodeEditorNode* FindNode(ImGui::NodeEditor::NodeId id);

    NodeEditorLink* FindLink(ImGui::NodeEditor::LinkId id);

    NodeEditorPin* FindPin(ImGui::NodeEditor::PinId id);

    bool IsPinLinked(ImGui::NodeEditor::PinId id);

    bool CanCreateLink(NodeEditorPin* a, NodeEditorPin* b);

    void BuildNode(NodeEditorNode* node);


    void BuildNodes();

    ImColor GetIconColor(NodeEditorVariantType type);

    void DrawPinIcon(const NodeEditorPin& pin, bool connected, int alpha);

    void ShowStyleEditor(bool* show = nullptr);

    void ShowLeftPane(float paneWidth);
    void Execute();

    NodeEditorIdAllocator m_IdAllocator;
    const int m_PinIconSize = 24;
    std::vector<Scope<NodeEditorNode>> m_Nodes;
    std::vector<NodeEditorLink> m_Links;
    ImTextureID m_HeaderBackground = 0;
    ImTextureID m_SaveIcon = 0;
    ImTextureID m_RestoreIcon = 0;

    Scope<::Aether::ImGuiComponent::Image> m_HeaderBackgroundImage = 0;
    Scope<::Aether::ImGuiComponent::Image> m_SaveIconImage = 0;
    Scope<::Aether::ImGuiComponent::Image> m_RestoreIconImage = 0;
    Scope<DeviceTexture> m_HeaderBackgroundDeviceImage = 0;
    Scope<DeviceTexture> m_SaveIconDeviceImage = 0;
    Scope<DeviceTexture> m_RestoreIconDeviceImage = 0;

    const float m_TouchTime = 1.0f;
    std::map<ImGui::NodeEditor::NodeId, float, NodeEditorNodeIdLess> m_NodeTouchTime;
    bool m_ShowOrdinals = false;

    ImGui::NodeEditor::EditorContext* m_Editor = nullptr;

private:
    ImGuiColorEditFlags edit_mode = ImGuiColorEditFlags_DisplayRGB;
    ImGuiTextFilter filter;
    bool showStyleEditor = false;
    int changeCount = 0;
    ImGui::NodeEditor::NodeId contextNodeId = 0;
    ImGui::NodeEditor::LinkId contextLinkId = 0;
    ImGui::NodeEditor::PinId contextPinId = 0;
    bool createNewNode = false;
    NodeEditorPin* newNodeLinkPin = nullptr;
    NodeEditorPin* newLinkPin = nullptr;
    float leftPaneWidth = 400.0f;
    float rightPaneWidth = 800.0f;
    char buffer[128] = "Edit Me\nMultiline!";
    bool wasActive = false;

private:
    std::vector<NodeEditorStep> m_Steps;
    bool m_CompileDirtyFlag = true;
    bool Compile();
private:

    std::unordered_map<std::string,NodeCreator> m_NodeCreators;
};
} // namespace Aether::ImGuiComponent
