#pragma once
#include "../BlueprintNode.h"
namespace Aether::ImGuiComponent::BlueprintTest
{
class MessageNode : public BlueprintNode
{
public:
    size_t MaxMessageLength = 256;
    MessageNode(BlueprintIdAllocator& idAllocator)
    {
        Type = BlueprintNodeType::Blueprint;
        Name = "Message";
        SetNodeId(idAllocator.NextId());
        AddOutputPin(idAllocator.NextId(), "string", BlueprintPinType::Flow);
        {
            auto& value = OutputValues.back();
            value = std::string(MaxMessageLength, 0);
        }

        InputColumnWidth = 0;
        LocalVariableColumnWidth = 0;
    }
    virtual std::optional<std::string> OnExecute(const std::vector<BlueprintInputPinValue>& inputs,
                                                 std::vector<BlueprintVariant>& outputs) override
    {
        return std::nullopt;
    }
    virtual void DrawOutput(ImGuiComponent::BlueprintPin& pin, ImGuiComponent::BlueprintVariant& value,
                            int index) override
    {
        ImGui::Text("Test Output %d", index);
        ImGuiEx::BlockPushLast();
    }

    virtual void DrawLocalVariable(int index) override
    {
        switch (index)
        {
        case 0: {
            auto& value = localVariables[0];
            if (!std::holds_alternative<std::string>(value))
            {
                value = std::string(MaxMessageLength, 0);
            }
            auto& s = std::get<std::string>(value);
            ImGui::InputText("Message", s.data(), MaxMessageLength);
        }
        break;

        default:
            assert(false && "Invalid local variable index");
            break;
        }
    }
};
} // namespace Aether::ImGuiComponent::BlueprintTest