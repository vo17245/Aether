#pragma once
#include <ImGui/NodeEditor/imgui_node_editor.h>
#include "BlueprintVariant.h"
#include <vector>
#include <optional>
#include "BlueprintIdAllocator.h"
#include <ImGui/ImGui.h>
namespace Aether::ImGuiComponent
{
struct BlueprintInputPinValue
{
    std::vector<BlueprintVariant*> values;
};

enum class BlueprintPinType
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

enum class BlueprintPinKind
{
    Output,
    Input
};

enum class BlueprintNodeType
{
    Blueprint,
    Simple,
    Tree,
    Comment,
    Houdini
};
struct BlueprintNode;

struct BlueprintPin
{
    ImGui::NodeEditor::PinId ID;
    BlueprintNode* Node; // node this pin belongs to
    std::string Name;
    BlueprintPinType Type;
    BlueprintPinKind Kind;
    BlueprintPin(int id, const char* name, BlueprintPinType type) :
        ID(id), Node(nullptr), Name(name), Type(type), Kind(BlueprintPinKind::Input)
    {
    }
};

struct BlueprintStep
{
    BlueprintNode* node;
    std::vector<BlueprintInputPinValue> inputValues;
};

struct BlueprintLink
{
    ImGui::NodeEditor::LinkId ID;

    ImGui::NodeEditor::PinId StartPinID;
    ImGui::NodeEditor::PinId EndPinID;

    ImColor Color;

    BlueprintLink(ImGui::NodeEditor::LinkId id, ImGui::NodeEditor::PinId startPinId,
                  ImGui::NodeEditor::PinId endPinId) :
        ID(id), StartPinID(startPinId), EndPinID(endPinId), Color(255, 255, 255)
    {
    }
};

struct BlueprintNodeIdLess
{
    bool operator()(const ImGui::NodeEditor::NodeId& lhs, const ImGui::NodeEditor::NodeId& rhs) const
    {
        return lhs.AsPointer() < rhs.AsPointer();
    }
};

class BlueprintNode
{
public:
    virtual ~BlueprintNode() = default;
    virtual std::optional<std::string> OnExecute(const std::vector<BlueprintInputPinValue>& inputs,
                                                 std::vector<BlueprintVariant>& outputs) = 0;
    virtual void DrawOutput(BlueprintPin& pin,BlueprintVariant& value,int index)=0;
    virtual void DrawLocalVariable(int index)=0;
    ImGui::NodeEditor::NodeId ID;
    std::string Name;
    std::vector<BlueprintPin> Inputs;
    std::vector<BlueprintPin> Outputs;
    std::vector<BlueprintVariant> OutputValues;
    ImColor Color;
    BlueprintNodeType Type;
    ImVec2 Size;
    std::vector<BlueprintVariant> localVariables;

    std::string State;
    std::string SavedState;
    uint32_t InputColumnWidth=150;
    uint32_t LocalVariableColumnWidth=0;
    void AddInputPin(int id,const std::string& name,BlueprintPinType type)
    {
        Inputs.emplace_back(id, name.c_str(), type);
        Inputs.back().Node = this;
        Inputs.back().Kind = BlueprintPinKind::Input;
    }
    void AddOutputPin(int id,const std::string& name,BlueprintPinType type)
    {
        Outputs.emplace_back(id, name.c_str(), type);
        Outputs.back().Node = this;
        Outputs.back().Kind = BlueprintPinKind::Output;
        OutputValues.emplace_back();
    }
    void SetNodeId(int id)
    {
        ID = ::ImGui::NodeEditor::NodeId(id);
    }
};
} // namespace Aether::ImGuiComponent