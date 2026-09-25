#pragma once
#include <cstdint>
#include <Render/Render.h>
#include <Window/Event.h>
#include <World/World.h>
namespace Aether
{

enum class SystemUpdatePhase : std::uint8_t
{
    EditorInput,
    EditorTools,
    Simulation,
    Presentation,
    EditorModel
};

constexpr std::uint8_t SystemUpdatePhaseOrder(SystemUpdatePhase phase) noexcept
{
    return static_cast<std::uint8_t>(phase);
}

class System
{
public:
    virtual std::string_view GetSignature() const = 0;
    virtual std::vector<std::string_view> GetDependencies() const = 0;
    virtual SystemUpdatePhase GetUpdatePhase() const noexcept { return SystemUpdatePhase::Simulation; }
    virtual void OnAttach(World* scene)
    {
    }
    virtual void OnUpdate(float deltaTime)
    {
    }
    virtual void OnUpdatePhase(SystemUpdatePhase phase, float deltaTime)
    {
        if (phase == GetUpdatePhase())
            OnUpdate(deltaTime);
    }
    virtual bool NeedRebuildRenderGraph()
    {
        return false;
    }
    virtual void OnBuildRenderGraph(RenderGraph::RenderGraph& renderGraph)
    {
    }
    virtual void OnUpload(PendingUploadList& uploadList)
    {
    }
    virtual ~System() = default;
    virtual void OnEvent(Event& event)
    {
    }
    virtual void OnFrameBegin(std::uint32_t frameSlot)
    {
    }
    virtual void ExtractRenderData(Render::RenderFeatureFrame& frame)
    {
        (void)frame;
    }
    virtual void OnDetach()
    {
    }
    // Called after a detached candidate registry replaces persistent World data.
    // Runtime systems use this safe-point hook to rebuild transient caches/services.
    virtual void OnWorldDataReplaced() noexcept
    {
    }
};
} // namespace Aether
