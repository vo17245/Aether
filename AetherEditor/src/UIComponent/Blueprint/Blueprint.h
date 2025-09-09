#pragma once
#include <ImGui/NodeEditor/imgui_node_editor.h>
#include <string>
#include <vector>
#include <functional>
#include <UIComponent/Image.h>
#include <variant>
#include "BlueprintNode.h"
#include "BlueprintIdAllocator.h"
namespace Aether::ImGuiComponent
{

class Blueprint
{
public:
    using NodeCreator = std::function<BlueprintNode*(BlueprintIdAllocator& idAllocator)>;
public:
    ~Blueprint()
    {
        Destroy();
    }

public:
    void RegisterNodeType(const std::string& name,NodeCreator&& creator)
    {
        m_NodeCreators[name] = std::move(creator);
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

    BlueprintNode* FindNode(ImGui::NodeEditor::NodeId id);

    BlueprintLink* FindLink(ImGui::NodeEditor::LinkId id);

    BlueprintPin* FindPin(ImGui::NodeEditor::PinId id);

    bool IsPinLinked(ImGui::NodeEditor::PinId id);

    bool CanCreateLink(BlueprintPin* a, BlueprintPin* b);

    void BuildNode(BlueprintNode* node);

    // BlueprintNode* SpawnInputActionNode();

    // BlueprintNode* SpawnBranchNode();

    // BlueprintNode* SpawnDoNNode();

    // BlueprintNode* SpawnOutputActionNode();

    // BlueprintNode* SpawnPrintStringNode();

    // BlueprintNode* SpawnMessageNode();

    // BlueprintNode* SpawnSetTimerNode();

    // BlueprintNode* SpawnLessNode();

    // BlueprintNode* SpawnWeirdNode();

    // BlueprintNode* SpawnTraceByChannelNode();

    // BlueprintNode* SpawnTreeSequenceNode();

    // BlueprintNode* SpawnTreeTaskNode();

    // BlueprintNode* SpawnTreeTask2Node();

    // BlueprintNode* SpawnComment();

    // BlueprintNode* SpawnHoudiniTransformNode();

    // BlueprintNode* SpawnHoudiniGroupNode();

    void BuildNodes();

    ImColor GetIconColor(BlueprintPinType type);

    void DrawPinIcon(const BlueprintPin& pin, bool connected, int alpha);

    void ShowStyleEditor(bool* show = nullptr);

    void ShowLeftPane(float paneWidth);
    void Execute();

    BlueprintIdAllocator m_IdAllocator;
    const int m_PinIconSize = 24;
    std::vector<Scope<BlueprintNode>> m_Nodes;
    std::vector<BlueprintLink> m_Links;
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
    std::map<ImGui::NodeEditor::NodeId, float, BlueprintNodeIdLess> m_NodeTouchTime;
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
    BlueprintPin* newNodeLinkPin = nullptr;
    BlueprintPin* newLinkPin = nullptr;
    float leftPaneWidth = 400.0f;
    float rightPaneWidth = 800.0f;
    char buffer[128] = "Edit Me\nMultiline!";
    bool wasActive = false;

private:
    std::vector<BlueprintStep> m_Steps;
    bool m_CompileDirtyFlag = true;
    bool Compile();
private:

    std::unordered_map<std::string,NodeCreator> m_NodeCreators;
};
} // namespace Aether::ImGuiComponent
