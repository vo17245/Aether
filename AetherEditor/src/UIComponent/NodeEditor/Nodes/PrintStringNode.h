#pragma once
#include "../NodeEditorNode.h"
#include <Debug/Log.h>
namespace Aether::ImGuiComponent::NodeEditorTest
{
class PrintStringNode:public NodeEditorNode
{
public:
    PrintStringNode(NodeEditorIdAllocator& idAllocator)
    {
        Name="PrintString";
        Type=NodeEditorNodeType::NodeEditor;
        SetNodeId(idAllocator.NextId());
        CreateInputPin(idAllocator.NextId());
    }
    virtual std::optional<std::string> OnExecute(const std::vector<NodeEditorVariant*>& inputs)
    {
        auto* inputText=inputs[0];
        if(inputText)
        {
            assert(std::holds_alternative<std::string*>(*inputText));
            auto text=std::get<std::string*>(*inputText);
            Debug::Log::Debug("[NodeEditor] {}", *text);
        }
        return std::nullopt;
    }
private:
    void CreateInputPin(int id)
    {
        AddInputPin<std::string*>(id,"InString");
    }

    
};
}