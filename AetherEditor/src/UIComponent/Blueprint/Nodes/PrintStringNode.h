#pragma once
#include "../BlueprintNode.h"
#include <Debug/Log.h>
namespace Aether::ImGuiComponent::BlueprintTest
{
class PrintStringNode:public BlueprintNode
{
public:
    PrintStringNode(BlueprintIdAllocator& idAllocator)
    {
        Type=BlueprintNodeType::Blueprint;
        Name="PrintString";
        SetNodeId(idAllocator.NextId());
        AddInputPin(idAllocator.NextId(), "string", BlueprintPinType::Flow);
        InputColumnWidth=0;
        LocalVariableColumnWidth=0;
    }
    virtual std::optional<std::string> OnExecute(const std::vector<BlueprintInputPinValue>& inputs,
                                                 std::vector<BlueprintVariant>& outputs) override
    {
        auto& input=inputs[0];
        for(auto* value:input.values)
        {
            if(std::holds_alternative<std::string>(*value))
            {
                Debug::Log::Debug("[Blueprint][PrintStringNode] {}", std::get<std::string>(*value));
            }
            else
            {
                return "Input is not string";
            }
        }
        return std::nullopt;
    }
    virtual void DrawOutput(ImGuiComponent::BlueprintPin& pin, ImGuiComponent::BlueprintVariant& value,
                            int index) override
    {
        ImGui::Text("Test Output %d", index);
        ImGuiEx::BlockPushLast();
    }

    virtual void DrawLocalVariable(int index)override
    {

    }
};
}