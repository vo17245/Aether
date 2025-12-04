#pragma once
namespace AetherEditor::ImGuiComponent
{
class NodeEditorIdAllocator
{
public:
    int NextId()
    {
        return m_NextId++;
    }

private:
    int m_NextId = 1;
};
} // namespace Aether::ImGuiComponent