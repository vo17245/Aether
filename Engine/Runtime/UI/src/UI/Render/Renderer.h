#pragma once
#include "Quad.h"
#include "RenderResource.h"
#include "Render/Scene/Camera2D.h"
#include <Render/RHI.h>
#include <Render/RenderGraph/RenderGraph.h>

namespace Aether::UI
{
class Renderer
{
public:
    Renderer(const Renderer&) = delete;
    Renderer(Renderer&&) = default;
    static std::expected<Renderer, std::string> Create(const RenderResource& resource)
    {
        Renderer renderer;
        renderer.m_RenderResource = resource;
        return renderer;
    }
    static Renderer CreateEmpty()
    {
        return Renderer();
    }
    void Begin(RenderGraph::RenderGraph& renderGraph, const RenderGraph::RenderPassDesc& renderPassDesc, const Camera2D& camera)
    {
        m_RenderGraph = &renderGraph;
        m_RenderPassDesc = renderPassDesc;
        m_Camera = &camera;
    }
    void DrawQuad(const Quad& quad)
    {
        (void)quad;
    }
    void End()
    {
        m_RenderGraph = nullptr;
        m_Camera = nullptr;
    }
    void Reset() {}
    void Clear() {}
    inline void OnFrameBegin()
    {
#ifdef AETHER_RUNTIME_CHECK
        m_IsBusy = false;
#endif
    }

private:
    Renderer() = default;
    RenderResource m_RenderResource;
    RenderGraph::RenderGraph* m_RenderGraph = nullptr;
    RenderGraph::RenderPassDesc m_RenderPassDesc{};
    const Camera2D* m_Camera = nullptr;
#ifdef AETHER_RUNTIME_CHECK
    bool m_IsBusy = false;
#endif
};
} // namespace Aether::UI
