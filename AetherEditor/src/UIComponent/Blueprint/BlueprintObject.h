#pragma once
namespace Aether::ImGuiComponent
{
enum class BlueprintObjectType
{
    Texture,
    Shader,
};
class BlueprintObject
{
public:
    virtual ~BlueprintObject() = default;
    BlueprintObject(BlueprintObjectType type) : m_Type(type)
    {
    }
    BlueprintObjectType GetType() const
    {
        return m_Type;
    }

private:
    BlueprintObjectType m_Type;
};
} // namespace Aether::ImGuiComponent