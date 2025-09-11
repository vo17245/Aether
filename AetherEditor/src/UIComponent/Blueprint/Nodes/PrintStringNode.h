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
        AddInputPin(idAllocator.NextId(), "", BlueprintPinType::Flow);
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
};
}