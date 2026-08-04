#pragma once
#include "../Component/Base.h"
#include "../Component/Quad.h"
#include "../../Render/Renderer.h"
#include "Render/Scene/Camera2D.h"
#include "System.h"
#include "../Component/Node.h"
#include <UI/Render/TextureCache.h>
#include <Core/Borrow.h>

namespace Aether::UI
{
class QuadSystem : public SystemI
{
public:
    Renderer* renderer = nullptr;
    void OnUpdate(float sec, World& scene) override
    {
        (void)sec;
        (void)scene;
    }
    void OnBuildRenderGraph(RenderGraph::RenderGraph& renderGraph,
                            const RenderGraph::RenderPassDesc& renderPassDesc,
                            Vec2f screenSize,
                            World& scene) override
    {
        (void)screenSize;
        assert(m_Camera && "camera is nullptr");
        if (!renderer)
        {
            return;
        }
        auto view = scene.Select<NodeComponent, BaseComponent, QuadComponent>();
        renderer->Begin(renderGraph, renderPassDesc, *m_Camera);
        for (const auto& [entity, node, base, quad] : view.each())
        {
            (void)entity;
            (void)node;
            if (!quad.visible)
            {
                continue;
            }
            quad.quad.SetPosition(Vec3f(base.position.x(), base.position.y(), base.z));
            quad.quad.SetSize(base.size);
            renderer->DrawQuad(quad.quad);
        }
        renderer->End();
    }
    void SetCamera(Camera2D* camera)
    {
        m_Camera = camera;
    }
    QuadSystem(Borrow<TextureCache> textureCache) : m_TextureCache(textureCache)
    {
    }

private:
    Camera2D* m_Camera = nullptr;
    Borrow<TextureCache> m_TextureCache;
};
} // namespace Aether::UI
