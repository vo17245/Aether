#pragma once
#include <functional>
#include <optional>
#include <Render/RHI.h>
#include <Render/RenderGraph/RenderGraph.h>

namespace Aether::UI
{
class FilterChain
{
public:
    using FilterI = std::function<std::optional<std::string>(rhi::Texture2D& from,
                                                             RenderGraph::RenderGraph& renderGraph,
                                                             const RenderGraph::RenderPassDesc& renderPassDesc)>;
    FilterChain() = default;
    void Push(const FilterI& filter)
    {
        m_Filters.push_back(filter);
    }
    std::optional<std::string> Render(rhi::Texture2D& from,
                                      RenderGraph::RenderGraph& renderGraph,
                                      const RenderGraph::RenderPassDesc& renderPassDesc)
    {
        for (auto& filter : m_Filters)
        {
            auto err = filter(from, renderGraph, renderPassDesc);
            if (err)
            {
                return err;
            }
        }
        return std::nullopt;
    }

private:
    std::vector<FilterI> m_Filters;
};
} // namespace Aether::UI
