#pragma once
#include <Render/RHI.h>
#include <Render/RenderGraph/RenderGraph.h>

namespace Aether::UI
{
class GammaFilter
{
public:
    static std::expected<GammaFilter, std::string> Create()
    {
        return GammaFilter();
    }
    bool Render(rhi::Texture2D& from, RenderGraph::RenderGraph& renderGraph, const RenderGraph::RenderPassDesc& renderPassDesc)
    {
        (void)from;
        (void)renderGraph;
        (void)renderPassDesc;
        return true;
    }
    void SetGamma(float gamma)
    {
        m_Gamma = gamma;
    }

private:
    GammaFilter() = default;
    float m_Gamma = 2.2f;
};
} // namespace Aether::UI
