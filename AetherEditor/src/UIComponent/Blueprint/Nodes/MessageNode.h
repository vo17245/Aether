#pragma once
#include "../BlueprintNode.h"
namespace Aether::ImGuiComponent::BlueprintTest
{
class MessageNode:public BlueprintNode
{
public:
    MessageNode(BlueprintIdAllocator& idAllocator)
    {
        Type=BlueprintNodeType::Blueprint;
        Name="Message";
        SetNodeId(idAllocator.NextId());
        AddInputPin(idAllocator.NextId(), "", BlueprintPinType::Flow);
    }
    virtual std::optional<std::string> OnExecute(const std::vector<BlueprintInputPinValue>& inputs,
                                                 std::vector<BlueprintVariant>& outputs) override
    {
        return std::nullopt;
    }
};
}