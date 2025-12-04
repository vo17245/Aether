#pragma once
#include <ImGui/NodeEditor/imgui_node_editor.h>
#include "NodeEditorVariant.h"
#include <vector>
#include <optional>
#include "NodeEditorIdAllocator.h"
#include <ImGui/ImGui.h>
#include "View.h"
#include <Core/Core.h>
namespace AetherEditor::ImGuiComponent
{

enum class NodeEditorPinType
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

enum class NodeEditorPinKind
{
    Output,
    Input
};

enum class NodeEditorNodeType
{
    NodeEditor,
    Simple,
    Tree,
    Comment,
    Houdini
};
struct NodeEditorNode;

struct NodeEditorPin
{
    ImGui::NodeEditor::PinId ID;
    NodeEditorNode* Node; // node this pin belongs to
    std::string Name;
    NodeEditorVariantType Type; // type index in NodeEditorVariant
    NodeEditorPinKind Kind;
    NodeEditorPin(int id, const std::string& name, NodeEditorVariantType type) :
        ID(id), Node(nullptr), Name(name), Type(type), Kind(NodeEditorPinKind::Input)
    {
    }
};

struct NodeEditorStep
{
    NodeEditorNode* node;
    std::vector<NodeEditorVariant*> inputValues;
};

struct NodeEditorLink
{
    ImGui::NodeEditor::LinkId ID;

    ImGui::NodeEditor::PinId StartPinID;
    ImGui::NodeEditor::PinId EndPinID;

    ImColor Color;

    NodeEditorLink(ImGui::NodeEditor::LinkId id, ImGui::NodeEditor::PinId startPinId,
                  ImGui::NodeEditor::PinId endPinId) :
        ID(id), StartPinID(startPinId), EndPinID(endPinId), Color(255, 255, 255)
    {
    }
};

struct NodeEditorNodeIdLess
{
    bool operator()(const ImGui::NodeEditor::NodeId& lhs, const ImGui::NodeEditor::NodeId& rhs) const
    {
        return lhs.AsPointer() < rhs.AsPointer();
    }
};

class NodeEditorNode
{
public:
    virtual ~NodeEditorNode() = default;
    virtual std::optional<std::string> OnExecute(const std::vector<NodeEditorVariant*>& inputs)
    {
        return std::nullopt;
    }
    ImGui::NodeEditor::NodeId ID;
    std::string Name;
    std::vector<NodeEditorPin> Inputs;
    std::vector<NodeEditorPin> Outputs;
    std::vector<NodeEditorVariant> OutputValues;
    ImColor Color;
    NodeEditorNodeType Type;
    ImVec2 Size;
    std::vector<Aether::Scope<View>> LocalVariableViews;

    std::string State;
    std::string SavedState;
    uint32_t InputColumnWidth = 0;
    uint32_t LocalVariableColumnWidth = 0;
    //===================== build node helper==================
    template <typename T>
    void AddInputPin(int id, const std::string& name)
    {
        Inputs.emplace_back(id, name, GetNodeEditorVariantTypeEnum<T>());
        Inputs.back().Node = this;
        Inputs.back().Kind = NodeEditorPinKind::Input;
    }
    template <typename T>
    void AddOutputPin(int id, const std::string& name, const T& value)
    {
        Outputs.emplace_back(id, name, GetNodeEditorVariantTypeEnum<T>());
        Outputs.back().Node = this;
        Outputs.back().Kind = NodeEditorPinKind::Output;
        OutputValues.emplace_back(NodeEditorVariant(value));
    }
    void SetNodeId(int id)
    {
        ID = ::ImGui::NodeEditor::NodeId(id);
    }
    //============================================
};
} // namespace Aether::ImGuiComponent