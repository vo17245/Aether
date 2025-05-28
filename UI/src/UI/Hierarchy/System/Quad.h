#pragma once
#include "../Component/Base.h"
#include "../Component/Quad.h"
#include "../../Render/Renderer.h"
#include "Render/Scene/Camera2D.h"
#include "System.h"
#include "../Component/Node.h"

namespace Aether::UI
{
class QuadSystem : public SystemI
{
public:
    ::Aether::UI::Renderer* renderer = nullptr;
    void OnUpdate(float sec, Scene& scene)
    {
        auto view = scene.Select<NodeComponent, BaseComponent, QuadComponent>();
    }
    void OnRender(DeviceCommandBufferView commandBuffer,
                  DeviceRenderPassView renderPass,
                  DeviceFrameBufferView frameBuffer,
                  Vec2f screenSize,
                  Scene& scene)
    {
        assert(m_Camera && "camera is nullptr");
        auto view = scene.Select<NodeComponent, BaseComponent, QuadComponent>();
        if (view.begin() == view.end())
        {
            return;
        }
        auto& renderer = *this->renderer;
        renderer.Begin(renderPass, frameBuffer, *m_Camera);
        bool anyQuad = false;
        for (const auto& [entity, node, base, quad] : view.each())
        {
            if (!quad.visible)
            {
                continue;
            }
            quad.quad.SetPosition(Vec3f(base.position.x(), base.position.y(), base.z));
            quad.quad.SetSize(base.size);
            renderer.DrawQuad(quad.quad);
            anyQuad = true;
        }
        if (!anyQuad)
        {
            renderer.Reset();
        }
        else
        {
            renderer.End(commandBuffer);
        }
    }
    void SetCamera(Camera2D* camera)
    {
        m_Camera = camera;
    }

private:
    Camera2D* m_Camera = nullptr; // not own
};
} // namespace Aether::UI