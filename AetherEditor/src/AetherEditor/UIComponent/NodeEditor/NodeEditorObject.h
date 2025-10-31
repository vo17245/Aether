#pragma once
namespace AetherEditor::ImGuiComponent
{
enum class NodeEditorObjectType
{
    Texture,
    Shader,
};
class NodeEditorObject
{
public:
    virtual ~NodeEditorObject() = default;
    NodeEditorObject(NodeEditorObjectType type) : m_Type(type)
    {
    }
    NodeEditorObjectType GetType() const
    {
        return m_Type;
    }

private:
    NodeEditorObjectType m_Type;
};
} // namespace Aether::ImGuiComponent