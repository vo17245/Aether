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
    ::Aether::UI::Renderer* renderer = nullptr;
    void OnUpdate(float sec, World& scene)
    {
        auto view = scene.Select<NodeComponent, BaseComponent, QuadComponent>();
    }
    void OnRender(DeviceCommandBufferView commandBuffer,
                  DeviceRenderPassView renderPass,
                  DeviceFrameBufferView frameBuffer,
                  Vec2f screenSize,
                  World& scene)
    {
        assert(m_Camera && "camera is nullptr");
        auto view = scene.Select<NodeComponent, BaseComponent, QuadComponent>();
        auto& renderer = *this->renderer;
        renderer.Begin( frameBuffer, *m_Camera);
        for (const auto& [entity, node, base, quad] : view.each())
        {
            if (!quad.visible)
            {
                continue;
            }
            quad.quad.SetPosition(Vec3f(base.position.x(), base.position.y(), base.z));
            quad.quad.SetSize(base.size);
            renderer.DrawQuad(quad.quad);
        }
        renderer.End(commandBuffer);
    }
    void SetCamera(Camera2D* camera)
    {
        m_Camera = camera;
    }
    QuadSystem(Borrow<TextureCache> textureCache)
        : m_TextureCache(textureCache)
    {
    }

private:
    Camera2D* m_Camera = nullptr; // not own
    Borrow<TextureCache> m_TextureCache;
};
} // namespace Aether::UI