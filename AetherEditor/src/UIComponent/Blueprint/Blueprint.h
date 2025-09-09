#pragma once
#include <ImGui/NodeEditor/imgui_node_editor.h>
#include <string>
#include <vector>
#include <functional>
#include <UIComponent/Image.h>
#include <variant>
namespace Aether::ImGuiComponent
{
enum class BlueprintObjectType
{
    Texture,
    Shader,
};
class BlueprintObject
{
public:
    virtual ~BlueprintObject()=default;
    BlueprintObject(BlueprintObjectType type):m_Type(type){}
    BlueprintObjectType GetType() const { return m_Type; }
private:
    BlueprintObjectType m_Type;
};
using BlueprintVariant = std::variant<std::monostate,int, float, std::string, bool, BlueprintObject*>;
class Blueprint
{
public:
    enum class PinType
    {
        Flow,
        Bool,
        Int,
        Float,
        String,
        Object,
        Function,
        Delegate,
    };

    enum class PinKind
    {
        Output,
        Input
    };

    enum class NodeType
    {
        Blueprint,
        Simple,
        Tree,
        Comment,
        Houdini
    };
    struct Node;

    struct Pin
    {
        ImGui::NodeEditor::PinId ID;
        Node* Node;// node this pin belongs to
        std::string Name;
        PinType Type;
        PinKind Kind;
        BlueprintVariant Value;
        Pin(int id, const char* name, PinType type) :
            ID(id), 
            Node(nullptr), 
            Name(name), Type(type), Kind(PinKind::Input)
        {
        }
    };
    struct Node
    {
        ImGui::NodeEditor::NodeId ID;
        std::string Name;
        std::vector<Pin> Inputs;
        std::vector<Pin> Outputs;
        ImColor Color;
        NodeType Type;
        ImVec2 Size;

        std::string State;
        std::string SavedState;

        Node(int id, const char* name, ImColor color = ImColor(255, 255, 255)) :
            ID(id), Name(name), Color(color), Type(NodeType::Blueprint), Size(0, 0)
        {
        }
        std::function<void(const std::vector<Pin>& inputs, const std::vector<Pin>& outputs)> onExecute;
        uint32_t inDegree=0;// for topological sort
    };

    struct Link
    {
        ImGui::NodeEditor::LinkId ID;

        ImGui::NodeEditor::PinId StartPinID;
        ImGui::NodeEditor::PinId EndPinID;

        ImColor Color;

        Link(ImGui::NodeEditor::LinkId id, ImGui::NodeEditor::PinId startPinId, ImGui::NodeEditor::PinId endPinId) :
            ID(id), StartPinID(startPinId), EndPinID(endPinId), Color(255, 255, 255)
        {
        }
    };

    struct NodeIdLess
    {
        bool operator()(const ImGui::NodeEditor::NodeId& lhs, const ImGui::NodeEditor::NodeId& rhs) const
        {
            return lhs.AsPointer() < rhs.AsPointer();
        }
    };
public:
    ~Blueprint()
    {
        Destroy();
    }
public:
    void Init();

    void Destroy();
    void Draw();

private:
    int GetNextId();

    ImGui::NodeEditor::LinkId GetNextLinkId();

    void TouchNode(ImGui::NodeEditor::NodeId id);

    float GetTouchProgress(ImGui::NodeEditor::NodeId id);

    void UpdateTouch();

    Node* FindNode(ImGui::NodeEditor::NodeId id);

    Link* FindLink(ImGui::NodeEditor::LinkId id);

    Pin* FindPin(ImGui::NodeEditor::PinId id);

    bool IsPinLinked(ImGui::NodeEditor::PinId id);

    bool CanCreateLink(Pin* a, Pin* b);

    void BuildNode(Node* node);

    Node* SpawnInputActionNode();

    Node* SpawnBranchNode();

    Node* SpawnDoNNode();

    Node* SpawnOutputActionNode();

    Node* SpawnPrintStringNode();

    Node* SpawnMessageNode();

    Node* SpawnSetTimerNode();

    Node* SpawnLessNode();

    Node* SpawnWeirdNode();

    Node* SpawnTraceByChannelNode();

    Node* SpawnTreeSequenceNode();

    Node* SpawnTreeTaskNode();

    Node* SpawnTreeTask2Node();

    Node* SpawnComment();

    Node* SpawnHoudiniTransformNode();

    Node* SpawnHoudiniGroupNode();

    void BuildNodes();

    ImColor GetIconColor(PinType type);

    void DrawPinIcon(const Pin& pin, bool connected, int alpha);

    void ShowStyleEditor(bool* show = nullptr);

    void ShowLeftPane(float paneWidth);
    void Execute();

    int m_NextId = 1;
    const int m_PinIconSize = 24;
    std::vector<Node> m_Nodes;
    std::vector<Link> m_Links;
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
    std::map<ImGui::NodeEditor::NodeId, float, NodeIdLess> m_NodeTouchTime;
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
    Pin* newNodeLinkPin = nullptr;
    Pin* newLinkPin = nullptr;
    float leftPaneWidth = 400.0f;
    float rightPaneWidth = 800.0f;
    char buffer[128] = "Edit Me\nMultiline!";
    bool wasActive = false;
private:
    std::vector<Node*> m_TopologicalSortedNodes;
    bool m_TopologicalSortDirty = true;
    void TopologicalSort();
};
}
