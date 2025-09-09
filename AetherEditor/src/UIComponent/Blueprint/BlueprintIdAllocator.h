#pragma once
namespace Aether::ImGuiComponent
{
class BlueprintIdAllocator
{
public:
    int NextId()
    {
        return m_NextId++;
    }

private:
    int m_NextId = 0;
};
} // namespace Aether::ImGuiComponent