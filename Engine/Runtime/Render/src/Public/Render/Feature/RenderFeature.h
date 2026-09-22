#pragma once

#include <Render/Frame/RenderFrameContext.h>
#include <Render/RenderGraph/RenderGraph.h>

#include <cstddef>
#include <concepts>
#include <cstdint>
#include <memory>
#include <stdexcept>
#include <utility>
#include <vector>

namespace Aether::Render
{
class RenderFeatureData
{
public:
    virtual ~RenderFeatureData() = default;
    virtual std::size_t PayloadBytes() const noexcept { return 0; }
};

struct RenderGraphBuildContext
{
    RenderGraph::RenderGraph& graph;
    RenderGraph::AccessId<rhi::Texture2D> output;
    std::uint32_t width = 0;
    std::uint32_t height = 0;
};

class RenderFeature : public std::enable_shared_from_this<RenderFeature>
{
public:
    virtual ~RenderFeature() = default;

    // These callbacks run on the render owner. P6 still invokes them from the
    // synchronous owner; P7 moves that owner to RenderThread without changing
    // the frontend contract.
    virtual void OnRenderAttach(RenderFrameContext&) {}
    virtual void OnRenderDetach(RenderFrameContext&) {}
    virtual void BuildRenderGraph(RenderGraphBuildContext&) {}
    virtual void PrepareFrame(RenderFrameContext&, const RenderFeatureData&) {}
};

struct RenderFeatureFrameEntry
{
    std::shared_ptr<RenderFeature> feature;
    std::unique_ptr<RenderFeatureData> data;
};

class RenderFeatureFrame
{
public:
    explicit RenderFeatureFrame(CpuFrameId cpuFrameId = 0) : m_CpuFrameId(cpuFrameId) {}

    RenderFeatureFrame(RenderFeatureFrame&&) noexcept = default;
    RenderFeatureFrame& operator=(RenderFeatureFrame&&) noexcept = default;
    RenderFeatureFrame(const RenderFeatureFrame&) = delete;
    RenderFeatureFrame& operator=(const RenderFeatureFrame&) = delete;

    void Add(std::shared_ptr<RenderFeature> feature, std::unique_ptr<RenderFeatureData> data)
    {
        if (!feature)
            throw std::invalid_argument("RenderFeatureFrame requires a feature");
        if (!data)
            throw std::invalid_argument("RenderFeatureFrame requires owned frame data");
        m_PayloadBytes += data->PayloadBytes();
        m_Entries.push_back({std::move(feature), std::move(data)});
    }

    template <typename Data, typename... Args>
        requires std::derived_from<Data, RenderFeatureData>
    void Emplace(std::shared_ptr<RenderFeature> feature, Args&&... args)
    {
        Add(std::move(feature), std::make_unique<Data>(std::forward<Args>(args)...));
    }

    void Prepare(RenderFrameContext& context) const
    {
        context.cpuFrameId = m_CpuFrameId;
        for (const auto& entry : m_Entries)
            entry.feature->PrepareFrame(context, *entry.data);
    }

    CpuFrameId GetCpuFrameId() const noexcept { return m_CpuFrameId; }
    std::size_t PayloadBytes() const noexcept { return m_PayloadBytes; }
    std::size_t Size() const noexcept { return m_Entries.size(); }
    bool Empty() const noexcept { return m_Entries.empty(); }

private:
    CpuFrameId m_CpuFrameId = 0;
    std::size_t m_PayloadBytes = 0;
    std::vector<RenderFeatureFrameEntry> m_Entries;
};
} // namespace Aether::Render
