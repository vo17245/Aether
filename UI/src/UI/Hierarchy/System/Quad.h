#pragma once
#include "../Component/Base.h"
#include "../Component/Quad.h"
#include "../../Render/Renderer.h"
#include "System.h"
#include "../Component/Node.h"

namespace Aether::UI
{
class QuadSystem:public SystemI
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
        auto view = scene.Select<NodeComponent, BaseComponent, QuadComponent>();
        if (view.begin() == view.end())
        {
            return;
        }
        auto& renderer = *this->renderer;
        renderer.Begin(renderPass, frameBuffer, screenSize);
        for (const auto& [entity, node, base, quad] : view.each())
        {
            quad.quad.SetPosition(base.position);
            quad.quad.SetSize(base.size);
            renderer.DrawQuad(quad.quad);
        }
        renderer.End(commandBuffer);
    }
};
} // namespace Aether::UI