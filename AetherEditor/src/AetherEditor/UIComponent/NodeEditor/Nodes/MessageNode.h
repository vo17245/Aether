#pragma once
#include "../NodeEditorNode.h"
#include "../View.h"
namespace AetherEditor::ImGuiComponent::NodeEditorTest
{
class MessageNode : public NodeEditorNode
{
public:
    size_t MaxMessageLength = 256;
    MessageNode(NodeEditorIdAllocator& idAllocator)
    {
        Type = NodeEditorNodeType::NodeEditor;
        Name = "Message";
        SetNodeId(idAllocator.NextId());
        InitInputBuffer();
        CreateInputView();
        CreateOutputPin(idAllocator.NextId());
        LocalVariableColumnWidth=300;
        InputColumnWidth=0;
    }
private:
    size_t m_InputBufferSize=256;

    void InitInputBuffer()
    {
        m_InputBuffer.clear();
        m_InputBuffer.resize(m_InputBufferSize,0);
    }
    void CreateInputView()
    {
        auto view=Aether::CreateScope<TextInputView>(m_InputBuffer.data(),m_InputBufferSize);
        LocalVariableViews.push_back(std::move(view));
    }
    void CreateOutputPin(int id)
    {
        AddOutputPin<std::string*>(id, "Text", &m_InputBuffer);
    }
    std::string m_InputBuffer;
};
} // namespace Aether::ImGuiComponent::NodeEditorTest